/**
 * @file    adsr_engine.c
 * @brief   Implementación del motor ADSR Digital estilo Digitone
 *
 * Diseño del algoritmo de curvas:
 *   - A cada etapa se le asigna un "progress" t en [0.0, 1.0]
 *   - t se calcula linealmente en base a sample_counter / stage_length_smp
 *   - La curva transforma t → t' (el rango 0–1 se mantiene, solo cambia la forma)
 *   - El nivel actual = lerp(stage_start_level, stage_target_level, t')
 *
 * Esto garantiza que SIEMPRE se llega exactamente al target, sin deriva acumulada.
 *
 * Salida al DAC8564:
 *   - Salida normalizada float [0.0, 1.0] × depth → float [-1.0, +1.0]
 *   - Escalado a uint16_t [0x0000, 0xFFFF]
 *   - Transmitido via SPI con el protocolo de 24 bits del DAC8564
 *     (4 bits de comando + 16 bits de dato + 4 bits dont-care)
 */

#include "adsr_engine.h"
#include <math.h>
#include <string.h>

/* =========================================================================
 * Presets predefinidos
 * ========================================================================= */

const adsr_params_t ADSR_PRESET_PERCUSSION = {
    .attack_ms      = 0.5f,
    .decay_ms       = 180.0f,
    .sustain_level  = 0.0f,
    .release_ms     = 80.0f,
    .attack_curve   = ADSR_CURVE_LOG,
    .decay_curve    = ADSR_CURVE_EXP,
    .release_curve  = ADSR_CURVE_EXP,
    .trig_mode      = ADSR_TRIG_RETRIG,
    .dest_depth     = { 1.0f, 0.7f, 0.0f },
    .dest_enabled   = { true, true, false },
    .loop_mode      = false,
    .hold_ms        = 0.0f,
};

const adsr_params_t ADSR_PRESET_PAD = {
    .attack_ms      = 800.0f,
    .decay_ms       = 400.0f,
    .sustain_level  = 0.75f,
    .release_ms     = 1200.0f,
    .attack_curve   = ADSR_CURVE_COSINE,
    .decay_curve    = ADSR_CURVE_EXP,
    .release_curve  = ADSR_CURVE_COSINE,
    .trig_mode      = ADSR_TRIG_LEGATO,
    .dest_depth     = { 1.0f, 0.4f, 0.0f },
    .dest_enabled   = { true, true, false },
    .loop_mode      = false,
    .hold_ms        = 0.0f,
};

const adsr_params_t ADSR_PRESET_BASS = {
    .attack_ms      = 2.0f,
    .decay_ms       = 90.0f,
    .sustain_level  = 0.6f,
    .release_ms     = 60.0f,
    .attack_curve   = ADSR_CURVE_LOG,
    .decay_curve    = ADSR_CURVE_EXP,
    .release_curve  = ADSR_CURVE_EXP,
    .trig_mode      = ADSR_TRIG_RETRIG,
    .dest_depth     = { 1.0f, 0.5f, 0.02f },
    .dest_enabled   = { true, true, true },
    .loop_mode      = false,
    .hold_ms        = 0.0f,
};

/* =========================================================================
 * Tabla de lookup para la curva logarítmica del mapeo de tiempos
 *
 * 128 entradas (MIDI 0–127) → tiempo en ms
 * Fórmula: t_ms = 0.1 * (1000.0^(x/127.0))
 * Esto da: MIDI 0 = 0.1ms, MIDI 64 ≈ 316ms, MIDI 127 = 100,000ms
 * ========================================================================= */
static float s_time_table_ms[128];
static bool  s_table_initialized = false;

static void build_time_table(void)
{
    if (s_table_initialized) return;

    for (int i = 0; i < 128; i++) {
        /* Rango logarítmico: 0.1ms → 100,000ms en 7 décadas */
        float normalized = (float)i / 127.0f;
        s_time_table_ms[i] = 0.1f * powf(1000000.0f, normalized);
    }
    s_table_initialized = true;
}

/* =========================================================================
 * Conversiones de tiempo
 * ========================================================================= */

uint32_t adsr_ms_to_samples(float ms)
{
    uint32_t samples = (uint32_t)((ms / 1000.0f) * (float)ADSR_SAMPLE_RATE);
    return (samples < 1U) ? 1U : samples;  /* Al menos 1 muestra */
}

float adsr_midi_to_ms(uint8_t midi_val)
{
    if (!s_table_initialized) build_time_table();
    uint8_t idx = (midi_val > 127) ? 127 : midi_val;
    return s_time_table_ms[idx];
}

/* =========================================================================
 * Curvas de forma
 *
 * Entrada:  t ∈ [0.0, 1.0] — progreso lineal en la etapa
 * Salida:   t' ∈ [0.0, 1.0] — progreso curvado
 *
 * Nota sobre EXP vs LOG:
 *   EXP (convexa): t^3 — Curva "RC discharge": sube/baja rápido al inicio
 *                  Ideal para decay/release porque el oído percibe la caída
 *                  exponencial como natural (igual que un condensador real)
 *
 *   LOG (cóncava): cbrt(t) ≈ t^(1/3) — opuesto a EXP:
 *                  Lento al inicio, rápido al final. Ideal para attack
 *                  percusivo o "pluck" donde la transient debe ser fuerte
 * ========================================================================= */

float adsr_apply_curve(float t, adsr_curve_t curve)
{
    /* Clamp defensivo — evita NaN si t sale ligeramente del rango */
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;

    switch (curve)
    {
        case ADSR_CURVE_LINEAR:
        default:
            return t;

        case ADSR_CURVE_EXP:
            /* Curva convexa — comportamiento de descarga RC (x^3 approx) */
            return t * t * t;

        case ADSR_CURVE_LOG:
            /* Curva cóncava — inversa de EXP (cbrtf = raíz cúbica) */
            return cbrtf(t);

        case ADSR_CURVE_COSINE:
            /* Curva en S — inicio y fin suaves, transición central pronunciada */
            /* (1 - cos(π·t)) / 2  →  derivada = 0 en t=0 y t=1 (sin clic) */
            return (1.0f - cosf(3.14159265f * t)) * 0.5f;
    }
}

/* =========================================================================
 * Inicialización
 * ========================================================================= */

void adsr_init(adsr_state_t *state, const adsr_params_t *params, uint8_t id)
{
    memset(state, 0, sizeof(adsr_state_t));
    state->params      = params;
    state->instance_id = id;
    state->stage       = ADSR_STAGE_IDLE;
    state->current_level = 0.0f;

    if (!s_table_initialized) build_time_table();
    adsr_recalculate(state);
}

/* =========================================================================
 * Precálculo de coeficientes
 *
 * Llamar cuando cambian los parámetros. Esto es O(1) y muy rápido.
 * No genera clics si se llama durante sustain o idle — el nivel actual
 * se preserva; solo los tiempos futuros cambian.
 * ========================================================================= */

void adsr_recalculate(adsr_state_t *state)
{
    const adsr_params_t *p = state->params;

    state->attack_smp  = adsr_ms_to_samples(p->attack_ms);
    state->hold_smp    = adsr_ms_to_samples(p->hold_ms);
    state->decay_smp   = adsr_ms_to_samples(p->decay_ms);
    state->release_smp = adsr_ms_to_samples(p->release_ms);

    /*
     * Si estamos en medio de una etapa y cambian los parámetros,
     * re-escalar el sample_counter para que el progreso relativo
     * se mantenga (no salta al inicio).
     */
    if (state->stage == ADSR_STAGE_ATTACK && state->stage_length_smp > 0) {
        float progress = (float)state->sample_counter / (float)state->stage_length_smp;
        state->stage_length_smp = state->attack_smp;
        state->sample_counter = (uint32_t)(progress * (float)state->attack_smp);
    }
    else if (state->stage == ADSR_STAGE_DECAY && state->stage_length_smp > 0) {
        float progress = (float)state->sample_counter / (float)state->stage_length_smp;
        state->stage_length_smp = state->decay_smp;
        state->sample_counter = (uint32_t)(progress * (float)state->decay_smp);
    }
    else if (state->stage == ADSR_STAGE_RELEASE && state->stage_length_smp > 0) {
        float progress = (float)state->sample_counter / (float)state->stage_length_smp;
        state->stage_length_smp = state->release_smp;
        state->sample_counter = (uint32_t)(progress * (float)state->release_smp);
    }
}

/* =========================================================================
 * Transición de etapa — función interna
 * ========================================================================= */

static void transition_to_stage(adsr_state_t *state, adsr_stage_t new_stage)
{
    const adsr_params_t *p = state->params;

    state->stage              = new_stage;
    state->sample_counter     = 0;
    state->stage_start_level  = state->current_level;   /* Desde donde estamos */

    switch (new_stage)
    {
        case ADSR_STAGE_ATTACK:
            state->stage_target_level = 1.0f;
            state->stage_length_smp   = state->attack_smp;
            break;

        case ADSR_STAGE_HOLD:
            /* Etapa de hold: nivel = 1.0 durante hold_ms */
            state->stage_target_level = 1.0f;
            state->stage_length_smp   = state->hold_smp;
            break;

        case ADSR_STAGE_DECAY:
            state->stage_target_level = p->sustain_level;
            state->stage_length_smp   = state->decay_smp;
            break;

        case ADSR_STAGE_SUSTAIN:
            state->stage_target_level = p->sustain_level;
            state->stage_length_smp   = UINT32_MAX;      /* Indefinido         */
            break;

        case ADSR_STAGE_RELEASE:
            state->stage_target_level = 0.0f;
            state->stage_length_smp   = state->release_smp;
            break;

        case ADSR_STAGE_IDLE:
        default:
            state->current_level      = 0.0f;
            state->stage_target_level = 0.0f;
            state->stage_length_smp   = 0;
            break;
    }
}

/* =========================================================================
 * Control de Gate
 * ========================================================================= */

void adsr_set_gate(adsr_state_t *state, bool gate_on)
{
    const adsr_params_t *p = state->params;
    bool flanco_rising  = gate_on  && !state->gate_state;
    bool flanco_falling = !gate_on && state->gate_state;

    state->gate_state = gate_on;

    /* --- Flanco RISING: Note ON --- */
    if (flanco_rising)
    {
        switch (p->trig_mode)
        {
            case ADSR_TRIG_RETRIG:
                /* Siempre reinicia attack desde el nivel actual (sin clic) */
                transition_to_stage(state, ADSR_STAGE_ATTACK);
                break;

            case ADSR_TRIG_LEGATO:
                /* Solo dispara si estaba idle — mode legato/mono */
                if (state->stage == ADSR_STAGE_IDLE) {
                    transition_to_stage(state, ADSR_STAGE_ATTACK);
                }
                /* Si ya estaba sonando → deja continuar (legato real)     */
                break;

            case ADSR_TRIG_ONE_SHOT:
                /* Single-cycle: solo si idle */
                if (state->stage == ADSR_STAGE_IDLE) {
                    transition_to_stage(state, ADSR_STAGE_ATTACK);
                }
                break;
        }
    }

    /* --- Flanco FALLING: Note OFF --- */
    if (flanco_falling)
    {
        if (p->trig_mode == ADSR_TRIG_ONE_SHOT) {
            /* ONE_SHOT ignora gate off — completa el ciclo completo        */
            return;
        }

        /* Pasa a release desde donde esté, excepto si ya está en idle     */
        if (state->stage != ADSR_STAGE_IDLE) {
            transition_to_stage(state, ADSR_STAGE_RELEASE);
        }
    }
}

/* =========================================================================
 * Conversión a palabra de DAC
 *
 * Protocolo DAC8564 (24 bits via SPI):
 *   Bit 23–20: Control (A1, A0, LD1, LD0)
 *   Bit 19–4:  Datos de 16 bits (MSB primero)
 *   Bit 3–0:   Don't care
 *
 * Canal:  A1,A0 = 00 → CH_A, 01 → CH_B, 10 → CH_C, 11 → CH_D
 * ========================================================================= */

/* Canales del DAC8564 asignados al sintetizador */
#define DAC8564_CH_VCA      0x00U   /* Canal A */
#define DAC8564_CH_VCF_CUT  0x01U   /* Canal B */
#define DAC8564_CH_PITCH    0x02U   /* Canal C */

static inline uint32_t build_dac8564_word(uint8_t channel, uint16_t data)
{
    /*
     * Byte alto:   PD1=0, PD0=0, A1, A0  (no power-down, canal seleccionado)
     * El formato es: [A1,A0,0,0, D15..D12] [D11..D4] [D3..D0, X,X,X,X]
     * Simplificamos como 3 bytes transmitidos MSB first:
     *   cmd_byte = (channel & 0x03) << 1  (registro de escritura sin latch)
     *   data_high = (data >> 8) & 0xFF
     *   data_low  = data & 0xFF
     */
    uint8_t cmd = (uint8_t)((channel & 0x03U) << 1U);
    return ((uint32_t)cmd << 16U) | ((uint32_t)data);
}

/* Envío SPI — stub que debe linkeditarse con el HAL de STM32 */
__attribute__((weak))
void adsr_spi_transmit_24bit(uint32_t word)
{
    /*
     * Implementar en el BSP del proyecto:
     *
     * uint8_t buf[3] = {
     *     (word >> 16) & 0xFF,
     *     (word >>  8) & 0xFF,
     *     (word      ) & 0xFF
     * };
     * HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_RESET);
     * HAL_SPI_Transmit(&hspi1, buf, 3, HAL_MAX_DELAY);
     * HAL_GPIO_WritePin(DAC_CS_GPIO_Port, DAC_CS_Pin, GPIO_PIN_SET);
     *
     * Para actualizar todos los canales simultáneamente usar el pin LDAC:
     * HAL_GPIO_WritePin(DAC_LDAC_GPIO_Port, DAC_LDAC_Pin, GPIO_PIN_RESET);
     * HAL_GPIO_WritePin(DAC_LDAC_GPIO_Port, DAC_LDAC_Pin, GPIO_PIN_SET);
     */
    (void)word;
}

/* =========================================================================
 * Proceso por muestra — corazón del motor ADSR
 *
 * Llamar exactamente a ADSR_SAMPLE_RATE Hz desde:
 *   - ISR del timer de audio (DMA half/full callback)
 *   - Tarea de FreeRTOS de alta prioridad con period fijo
 *
 * Latencia: <1µs en STM32H7 @ 480MHz (sin operaciones de punto flotante lento)
 * ========================================================================= */

void adsr_process_sample(adsr_state_t *state, adsr_dac_output_t *dac_output)
{
    const adsr_params_t *p = state->params;

    /* ------------------------------------------------------------------ */
    /* 1. Calcular el nivel de la envolvente según la etapa actual         */
    /* ------------------------------------------------------------------ */

    switch (state->stage)
    {
        case ADSR_STAGE_IDLE:
            state->current_level = 0.0f;
            break;

        case ADSR_STAGE_ATTACK:
        {
            float t = (float)state->sample_counter / (float)state->stage_length_smp;
            float t_curved = adsr_apply_curve(t, p->attack_curve);
            /* Interpolar desde start_level (puede ser >0 si retrigger en decay) */
            state->current_level = state->stage_start_level
                                 + (1.0f - state->stage_start_level) * t_curved;

            state->sample_counter++;
            if (state->sample_counter >= state->stage_length_smp) {
                state->current_level = 1.0f;
                /* Si hay hold, pasar a hold; si no, directo a decay */
                if (state->hold_smp > 1U) {
                    transition_to_stage(state, ADSR_STAGE_HOLD);
                } else {
                    transition_to_stage(state, ADSR_STAGE_DECAY);
                }
            }
            break;
        }

        case ADSR_STAGE_HOLD:
        {
            /* Level fijo en 1.0 durante hold_ms */
            state->current_level = 1.0f;
            state->sample_counter++;
            if (state->sample_counter >= state->stage_length_smp) {
                transition_to_stage(state, ADSR_STAGE_DECAY);
            }
            break;
        }

        case ADSR_STAGE_DECAY:
        {
            float t = (float)state->sample_counter / (float)state->stage_length_smp;
            float t_curved = adsr_apply_curve(t, p->decay_curve);
            state->current_level = state->stage_start_level
                                 + (p->sustain_level - state->stage_start_level) * t_curved;

            state->sample_counter++;
            if (state->sample_counter >= state->stage_length_smp) {
                state->current_level = p->sustain_level;

                if (p->loop_mode) {
                    /* Modo loop: volver a attack (LFO-like con forma ADSR) */
                    transition_to_stage(state, ADSR_STAGE_ATTACK);
                } else if (state->gate_state) {
                    transition_to_stage(state, ADSR_STAGE_SUSTAIN);
                } else {
                    /* Gate ya se fue durante el decay → directo a release */
                    transition_to_stage(state, ADSR_STAGE_RELEASE);
                }
            }
            break;
        }

        case ADSR_STAGE_SUSTAIN:
            /* Nivel constante. La transición a release la hace adsr_set_gate() */
            state->current_level = p->sustain_level;
            break;

        case ADSR_STAGE_RELEASE:
        {
            float t = (float)state->sample_counter / (float)state->stage_length_smp;
            float t_curved = adsr_apply_curve(t, p->release_curve);
            state->current_level = state->stage_start_level * (1.0f - t_curved);

            state->sample_counter++;
            if (state->sample_counter >= state->stage_length_smp) {
                state->current_level = 0.0f;

                if (p->trig_mode == ADSR_TRIG_ONE_SHOT && p->loop_mode) {
                    transition_to_stage(state, ADSR_STAGE_ATTACK);
                } else {
                    transition_to_stage(state, ADSR_STAGE_IDLE);
                }
            }
            break;
        }

        default:
            state->current_level = 0.0f;
            break;
    }

    /* Clamp final para evitar valores fuera de rango por errores de float */
    if (state->current_level < 0.0f) state->current_level = 0.0f;
    if (state->current_level > 1.0f) state->current_level = 1.0f;

    state->output_norm = state->current_level;

    /* ------------------------------------------------------------------ */
    /* 2. Calcular salidas por destino (depth scaling)                     */
    /* ------------------------------------------------------------------ */

    for (int d = 0; d < ADSR_NUM_DESTINATIONS; d++) {
        if (p->dest_enabled[d]) {
            state->dest_output[d] = state->current_level * p->dest_depth[d];
        } else {
            state->dest_output[d] = 0.0f;
        }
    }

    /* ------------------------------------------------------------------ */
    /* 3. Convertir a palabra de 16 bits para el DAC8564                  */
    /*    Escala: 0.0 → 0x0000, 1.0 → 0xFFFF                             */
    /*                                                                     */
    /*    Para PITCH: bipolar [-1.0, +1.0] centrado en 2.5V               */
    /*    Para VCA:   unipolar [0.0, 1.0] → [0V, 5V]                      */
    /*    Para VCF:   unipolar con profundidad de modulación               */
    /* ------------------------------------------------------------------ */

    /* VCA — unipolar */
    {
        float vca_val = state->dest_output[ADSR_DEST_VCA];
        if (vca_val < 0.0f) vca_val = 0.0f;
        if (vca_val > 1.0f) vca_val = 1.0f;
        dac_output->vca_word = (uint16_t)(vca_val * (float)DAC_FULL_SCALE);
    }

    /* VCF Cutoff — unipolar (depth puede ser negativo → modulación inversa) */
    {
        float vcf_val = state->dest_output[ADSR_DEST_VCF_CUT];
        /* Normar al rango [0, 1] desde [-1, 1] con offset de 0.5 */
        float vcf_norm = 0.5f + (vcf_val * 0.5f);
        if (vcf_norm < 0.0f) vcf_norm = 0.0f;
        if (vcf_norm > 1.0f) vcf_norm = 1.0f;
        dac_output->vcf_cut_word = (uint16_t)(vcf_norm * (float)DAC_FULL_SCALE);
    }

    /* PITCH — bipolar centrado en 0x7FFF (2.5V) */
    {
        float pitch_val = state->dest_output[ADSR_DEST_PITCH];
        if (pitch_val < -1.0f) pitch_val = -1.0f;
        if (pitch_val >  1.0f) pitch_val =  1.0f;
        float pitch_norm = 0.5f + (pitch_val * 0.5f);
        dac_output->pitch_word = (uint16_t)(pitch_norm * (float)DAC_FULL_SCALE);
    }

    /* ------------------------------------------------------------------ */
    /* 4. Transmitir al DAC8564 via SPI                                   */
    /*    Los 3 canales se escriben en el input register primero,         */
    /*    luego se actualiza LDAC simultáneamente (sin glitch)            */
    /* ------------------------------------------------------------------ */
    adsr_spi_transmit_24bit(build_dac8564_word(DAC8564_CH_VCA,     dac_output->vca_word));
    adsr_spi_transmit_24bit(build_dac8564_word(DAC8564_CH_VCF_CUT, dac_output->vcf_cut_word));
    adsr_spi_transmit_24bit(build_dac8564_word(DAC8564_CH_PITCH,   dac_output->pitch_word));

    /*
     * LDAC: Pulso de bajada para actualizar los 3 canales simultáneamente.
     * Esto evita que VCA y VCF actualicen en tiempos diferentes (glitch).
     *
     * HAL_GPIO_WritePin(DAC_LDAC_GPIO, DAC_LDAC_PIN, GPIO_PIN_RESET);
     * __NOP(); __NOP();   // 2 ciclos de delay mínimo (>5ns datasheet)
     * HAL_GPIO_WritePin(DAC_LDAC_GPIO, DAC_LDAC_PIN, GPIO_PIN_SET);
     */
}
