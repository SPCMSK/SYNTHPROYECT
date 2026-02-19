/**
 * @file    synth_voice.c
 * @brief   Integración del motor ADSR en el contexto de una voz de sintetizador
 *
 * Este archivo muestra cómo conectar el motor ADSR al pipeline de audio
 * en un STM32H7 con FreeRTOS.
 *
 * Diagrama de flujo:
 *
 *   MIDI NoteOn/Off
 *         │
 *         ▼
 *   adsr_set_gate()       ← Desde tarea MIDI (prioridad media)
 *         │
 *         ▼
 *   ISR DMA Audio (44100Hz)
 *         │
 *         ├── adsr_process_sample() → dac_output → SPI → DAC8564
 *         │         │
 *         │    (nivel float)
 *         │         │
 *         └── fm_synth_tick() × nivel → I2S → PCM5242 → SALIDA AUDIO
 *
 * Separación de dominios:
 *   - ADSR controla VCA/VCF analógicos via DAC8564 (SPI1)
 *   - Señal de audio digital va por I2S al PCM5242 (SAI1)
 *   - Ambos se sincronizan por el mismo timer base (TIM2 @ 44100Hz)
 */

#include "adsr_engine.h"
/* #include "stm32h7xx_hal.h" */   /* Descomentar en el proyecto real */
/* #include "fm_engine.h"      */   /* Motor FM — archivo separado      */

/* =========================================================================
 * Configuración del sistema de voces
 * ========================================================================= */

#define NUM_VOICES   4      /* Polifonía: 4 voces simultáneas              */

/* Pool de estados ADSR — una instancia por voz */
static adsr_state_t     s_voices[NUM_VOICES];

/* Parámetros modificables desde la UI / presets (en RAM) */
static adsr_params_t    s_voice_params[NUM_VOICES];

/* Salidas DAC (se usan como buffer interno antes del SPI) */
static adsr_dac_output_t s_dac_outputs[NUM_VOICES];

/* =========================================================================
 * Inicialización del sistema de voces
 * ========================================================================= */

void synth_voices_init(void)
{
    for (int v = 0; v < NUM_VOICES; v++) {
        /*
         * Copiar preset por defecto en RAM para que sea modificable.
         * En un sistema con presets: cargar desde flash QSPI aquí.
         */
        s_voice_params[v] = ADSR_PRESET_PERCUSSION;

        /* Inicializar el ADSR de cada voz */
        adsr_init(&s_voices[v], &s_voice_params[v], (uint8_t)v);
    }
}

/* =========================================================================
 * Ejemplo: Recepción de nota MIDI
 *
 * En un sistema real, esto viene del parser MIDI (USB o DIN-5).
 * La asignación de voz (voice stealing) se maneja aquí.
 * ========================================================================= */

static int8_t s_voice_notes[NUM_VOICES];      /* Nota MIDI asignada a c/voz */
static int8_t s_voice_next = 0;               /* Round-robin simple          */

void synth_note_on(uint8_t note, uint8_t velocity)
{
    /* Voice allocation round-robin (simplificado) */
    int v = s_voice_next;
    s_voice_next = (s_voice_next + 1) % NUM_VOICES;

    s_voice_notes[v] = (int8_t)note;

    /*
     * Escalar velocity a depth del VCA (0–127 → 0.0–1.0)
     * Esto da dinámica real: velocity suave = menos amplitud
     */
    s_voice_params[v].dest_depth[ADSR_DEST_VCA] = (float)velocity / 127.0f;
    adsr_recalculate(&s_voices[v]);

    /* Disparar la envolvente */
    adsr_set_gate(&s_voices[v], true);

    /*
     * Aquí también se configuraría el oscilador FM con la nota:
     * fm_set_note(&s_fm_voices[v], note);
     */
}

void synth_note_off(uint8_t note)
{
    /* Buscar la voz que tiene esa nota y soltar el gate */
    for (int v = 0; v < NUM_VOICES; v++) {
        if (s_voice_notes[v] == (int8_t)note) {
            adsr_set_gate(&s_voices[v], false);
            s_voice_notes[v] = -1;
            break;
        }
    }
}

/* =========================================================================
 * ISR de Audio — se llama exactamente a 44100 Hz
 *
 * En STM32H7 con I2S/SAI + DMA, esto se activa en el callback:
 *   HAL_SAI_TxHalfCpltCallback() y HAL_SAI_TxCpltCallback()
 *
 * Instrucciones de uso en FreeRTOS:
 *   - Esta función NO debe llamar a funciones de FreeRTOS que bloqueen
 *   - Usar xQueueSendFromISR si necesita comunicarse con otras tareas
 *   - El procesamiento ADSR es determinista y cabe en la ISR (<5µs)
 * ========================================================================= */

void audio_process_callback(int16_t *audio_buffer, uint32_t num_samples)
{
    for (uint32_t s = 0; s < num_samples; s++)
    {
        float mixed_sample = 0.0f;

        for (int v = 0; v < NUM_VOICES; v++)
        {
            /* 1. Calcular ADSR y actualizar DACs analógicos */
            adsr_process_sample(&s_voices[v], &s_dac_outputs[v]);

            /*
             * 2. Obtener muestra del oscilador FM (normalizada -1.0 a +1.0)
             *    En el hardware real, el DAC8564 ya controla el VCA analógico,
             *    por lo que aquí NO aplicamos el nivel de envolvente a la muestra
             *    digital — el VCA analógico lo hará en la ruta de señal.
             *
             *    EXCEPCIÓN: Modo "Digital FM puro" (bypass VCF/VCA analógico):
             *    En ese modo SÍ se aplica el nivel aquí para modular la amplitud
             *    digitalmente antes del PCM5242.
             */

            /* float fm_sample = fm_synth_tick(&s_fm_voices[v]); */
            float fm_sample = 0.0f; /* Placeholder — reemplazar con FM engine */

            /*
             * 3. Modo Híbrido: aplicar envolvente digitalmente como backup
             *    o como modulación de la señal FM antes del filtro analógico.
             *    Útil para inyectar la señal FM al VCF con nivel controlado.
             */
            fm_sample *= s_voices[v].output_norm;

            mixed_sample += fm_sample;
        }

        /* Normalizar por número de voces para no saturar */
        mixed_sample /= (float)NUM_VOICES;

        /* Convertir a int16 para el buffer I2S del PCM5242 */
        int32_t pcm = (int32_t)(mixed_sample * 32767.0f);
        if (pcm >  32767) pcm =  32767;
        if (pcm < -32768) pcm = -32768;

        audio_buffer[s * 2]     = (int16_t)pcm;   /* Canal L */
        audio_buffer[s * 2 + 1] = (int16_t)pcm;   /* Canal R */
    }
}

/* =========================================================================
 * Control de parámetros en tiempo real (desde UI / encoders / MIDI CC)
 *
 * CRÍTICO: adsr_recalculate() está diseñado para ser llamado en tiempo real
 * sin generar clics de audio. Los tiempos de las etapas en curso se
 * re-escalan proporcionalmente para preservar el progreso.
 * ========================================================================= */

/**
 * @brief Modifica el tiempo de attack de una voz en tiempo real
 * @param voice     Índice de voz (0–NUM_VOICES-1)
 * @param midi_val  Valor MIDI 0–127 → 0.1ms–100s (escala logarítmica)
 */
void synth_set_attack(uint8_t voice, uint8_t midi_val)
{
    if (voice >= NUM_VOICES) return;
    s_voice_params[voice].attack_ms = adsr_midi_to_ms(midi_val);
    adsr_recalculate(&s_voices[voice]);
}

/**
 * @brief Modifica el tiempo de decay
 */
void synth_set_decay(uint8_t voice, uint8_t midi_val)
{
    if (voice >= NUM_VOICES) return;
    s_voice_params[voice].decay_ms = adsr_midi_to_ms(midi_val);
    adsr_recalculate(&s_voices[voice]);
}

/**
 * @brief Modifica el nivel de sustain (0–127 → 0.0–1.0)
 */
void synth_set_sustain(uint8_t voice, uint8_t midi_val)
{
    if (voice >= NUM_VOICES) return;
    s_voice_params[voice].sustain_level = (float)midi_val / 127.0f;
    adsr_recalculate(&s_voices[voice]);
}

/**
 * @brief Modifica el tiempo de release
 */
void synth_set_release(uint8_t voice, uint8_t midi_val)
{
    if (voice >= NUM_VOICES) return;
    s_voice_params[voice].release_ms = adsr_midi_to_ms(midi_val);
    adsr_recalculate(&s_voices[voice]);
}

/**
 * @brief Cambia la curva del decay — el "character knob" del sonido
 * @param curve ADSR_CURVE_LINEAR / EXP / LOG / COSINE
 */
void synth_set_decay_curve(uint8_t voice, adsr_curve_t curve)
{
    if (voice >= NUM_VOICES) return;
    s_voice_params[voice].decay_curve = curve;
    /* No necesita recalculate() — la curva se aplica en tiempo real */
}

/**
 * @brief Profundidad de modulación de la envolvente sobre el VCF
 * @param depth  -1.0 (inverso) → 0.0 (nulo) → +1.0 (máximo)
 */
void synth_set_vcf_env_depth(uint8_t voice, float depth)
{
    if (voice >= NUM_VOICES) return;
    if (depth < -1.0f) depth = -1.0f;
    if (depth >  1.0f) depth =  1.0f;
    s_voice_params[voice].dest_depth[ADSR_DEST_VCF_CUT] = depth;
}

/* =========================================================================
 * Guardado y carga de presets (hacia/desde Flash QSPI W25Q128)
 *
 * La estructura adsr_params_t es directamente serializable (sin punteros).
 * Tamaño: ~48 bytes por preset. Con W25Q128 (128Mbit = 16MB) → 327,680 presets
 * ========================================================================= */

/**
 * @brief Serializa los parámetros de una voz para guardar como preset
 * @param voice         Índice de voz fuente
 * @param out_buffer    Buffer de destino (mínimo sizeof(adsr_params_t) bytes)
 * @return              Número de bytes escritos
 */
uint32_t synth_serialize_preset(uint8_t voice, uint8_t *out_buffer)
{
    if (voice >= NUM_VOICES || out_buffer == 0) return 0;
    uint32_t size = sizeof(adsr_params_t);
    /* memcpy(out_buffer, &s_voice_params[voice], size); */
    (void)out_buffer;
    return size;
}

/**
 * @brief Carga un preset en una voz desde un buffer deserializado
 */
void synth_load_preset(uint8_t voice, const uint8_t *in_buffer)
{
    if (voice >= NUM_VOICES || in_buffer == 0) return;
    /* memcpy(&s_voice_params[voice], in_buffer, sizeof(adsr_params_t)); */
    (void)in_buffer;
    adsr_recalculate(&s_voices[voice]);
}
