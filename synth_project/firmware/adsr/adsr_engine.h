/**
 * @file    adsr_engine.h
 * @brief   Motor ADSR Digital estilo Digitone para STM32H7
 *
 * Características implementadas:
 *  - Curvas configurables por etapa: lineal, exponencial, logarítmica
 *  - Tiempos con escala logarítmica (0.1ms → 100s)
 *  - Múltiples destinos: VCA, VCF Cutoff, Pitch (con depth independiente)
 *  - Modos de retriggering: RETRIG, LEGATO, ONE_SHOT
 *  - Resolución de 32-bit float para cálculo interno, salida 16-bit al DAC
 *  - Anti-click: transición suave cuando se retrigger en mitad de un decay
 *
 * Frecuencia de control recomendada: 44100 Hz (a tasa de audio)
 * Se puede reducir a 1000 Hz para ahorrar CPU si no hay modulaciones de audio rate.
 */

#ifndef ADSR_ENGINE_H
#define ADSR_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================================
 * Constantes globales
 * ========================================================================= */

#define ADSR_SAMPLE_RATE        44100U      /* Hz — frecuencia de cálculo   */
#define ADSR_NUM_INSTANCES      8U          /* Voces simultáneas máximas    */
#define ADSR_NUM_DESTINATIONS   3U          /* VCA, VCF_CUT, PITCH          */

/* Rango de salida del DAC8564: 0x0000 → 0xFFFF representa 0V → 5V */
#define DAC_FULL_SCALE          0xFFFFU
#define DAC_VCA_MAX_V           5.0f        /* Voltios a escala completa    */
#define DAC_VCF_MAX_V           5.0f
#define DAC_PITCH_CENTER_V      2.5f        /* 2.5V = pitch sin modulación  */

/* =========================================================================
 * Tipos enumerados
 * ========================================================================= */

/**
 * @brief Estado de la máquina de estados del ADSR
 */
typedef enum {
    ADSR_STAGE_IDLE     = 0,  /* Env a 0, no procesando                  */
    ADSR_STAGE_ATTACK   = 1,  /* Subida desde nivel actual → 1.0          */
    ADSR_STAGE_DECAY    = 2,  /* Bajada desde 1.0 → sustain level         */
    ADSR_STAGE_SUSTAIN  = 3,  /* Nivel constante mientras gate=HIGH       */
    ADSR_STAGE_RELEASE  = 4,  /* Bajada desde nivel actual → 0            */
} adsr_stage_t;

/**
 * @brief Forma de curva por etapa — igual que el Curve knob del Digitone
 *
 *  LINEAR:  Cambio constante por muestra
 *  EXP:     Curva convexa — sube/baja rápido al inicio, lento al final
 *           Más musical para decay y release (simula condensadores RC)
 *  LOG:     Curva cóncava — lento al inicio, rápido al final
 *           Ideal para attack percusivo (slap)
 *  COSINE:  Curva en S suave — atacado y liberado muy gradual
 */
typedef enum {
    ADSR_CURVE_LINEAR  = 0,
    ADSR_CURVE_EXP     = 1,   /* x^(1/3) aproximado con lookup table      */
    ADSR_CURVE_LOG     = 2,   /* x^3     — respuesta logarítmica          */
    ADSR_CURVE_COSINE  = 3,   /* (1 - cos(pi*x)) / 2                      */
} adsr_curve_t;

/**
 * @brief Comportamiento al recibir gate ON mientras el env está activo
 */
typedef enum {
    ADSR_TRIG_RETRIG  = 0,   /* Reinicia desde nivel actual (default)    */
    ADSR_TRIG_LEGATO  = 1,   /* Solo retrigger en etapa IDLE             */
    ADSR_TRIG_ONE_SHOT= 2,   /* Ignora gate OFF hasta terminar release   */
} adsr_trig_mode_t;

/**
 * @brief Índice de destino de modulación
 */
typedef enum {
    ADSR_DEST_VCA      = 0,
    ADSR_DEST_VCF_CUT  = 1,
    ADSR_DEST_PITCH    = 2,
} adsr_dest_t;

/* =========================================================================
 * Estructuras de parámetros — editables por el usuario/preset
 * ========================================================================= */

/**
 * @brief Parámetros completos de una envolvente
 *
 * Todos los tiempos están en milisegundos.
 * La resolución es logarítmica: los valores de 0–127 se mapean a 0.1ms–100,000ms
 * usando la misma escala que el Digitone.
 */
typedef struct {
    float           attack_ms;      /* 0.1 ms → 100,000 ms                */
    float           decay_ms;       /* 0.1 ms → 100,000 ms                */
    float           sustain_level;  /* 0.0 → 1.0 (normalizado)            */
    float           release_ms;     /* 0.1 ms → 100,000 ms                */

    adsr_curve_t    attack_curve;   /* Curva de la etapa de ataque         */
    adsr_curve_t    decay_curve;    /* Curva de la etapa de decay          */
    adsr_curve_t    release_curve;  /* Curva de la etapa de release        */

    adsr_trig_mode_t trig_mode;

    /* Destinos y profundidades de modulación */
    float           dest_depth[ADSR_NUM_DESTINATIONS]; /* -1.0 → +1.0     */
    bool            dest_enabled[ADSR_NUM_DESTINATIONS];

    /* Opciones avanzadas */
    bool            loop_mode;      /* Loop A→D como LFO cuando true       */
    float           hold_ms;        /* Etapa de hold entre A y D           */
} adsr_params_t;

/* =========================================================================
 * Estructura de estado interno — no modificar directamente
 * ========================================================================= */

typedef struct {
    /* Estado de la máquina */
    adsr_stage_t    stage;
    float           current_level;      /* Nivel normalizado actual 0.0–1.0         */
    float           stage_start_level;  /* Nivel al inicio de la etapa actual       */
    float           stage_target_level; /* Nivel objetivo de la etapa actual        */
    uint32_t        sample_counter;     /* Muestras transcurridas en etapa actual   */
    uint32_t        stage_length_smp;   /* Duración de la etapa en muestras         */

    /* Coeficientes precalculados al cambiar parámetros */
    uint32_t        attack_smp;
    uint32_t        hold_smp;
    uint32_t        decay_smp;
    uint32_t        release_smp;

    /* Gate */
    bool            gate_state;         /* Estado actual del gate (on/off)          */
    bool            gate_prev;          /* Estado anterior para flanco              */

    /* Salidas calculadas (normalizado) para cada destino */
    float           output_norm;        /* Nivel de envolvente 0.0–1.0              */
    float           dest_output[ADSR_NUM_DESTINATIONS]; /* Escalado por depth       */

    /* Referencia a parámetros (puntero — permite cambio en tiempo real) */
    const adsr_params_t *params;

    /* ID de instancia */
    uint8_t         instance_id;
} adsr_state_t;

/* =========================================================================
 * Estructura de salida hacia el DAC — producida por el motor, consumida por SPI
 * ========================================================================= */

typedef struct {
    uint16_t vca_word;      /* Palabra de 16 bits para DAC canal VCA       */
    uint16_t vcf_cut_word;  /* Palabra de 16 bits para DAC canal VCF CUT   */
    uint16_t pitch_word;    /* Palabra de 16 bits para DAC canal PITCH      */
} adsr_dac_output_t;

/* =========================================================================
 * API Pública
 * ========================================================================= */

/**
 * @brief Inicializa una instancia de ADSR con parámetros por defecto
 * @param state     Puntero a la estructura de estado a inicializar
 * @param params    Puntero a los parámetros (puede ser preset guardado)
 * @param id        Identificador de voz (0–ADSR_NUM_INSTANCES-1)
 */
void adsr_init(adsr_state_t *state, const adsr_params_t *params, uint8_t id);

/**
 * @brief Precalcula coeficientes internos cuando cambian los parámetros
 *        Llamar siempre que se modifique adsr_params_t (ej: cambio de preset)
 * @param state     Estado de la instancia
 */
void adsr_recalculate(adsr_state_t *state);

/**
 * @brief Actualiza el estado del gate
 * @param state     Estado de la instancia
 * @param gate_on   true = nota ON, false = nota OFF
 */
void adsr_set_gate(adsr_state_t *state, bool gate_on);

/**
 * @brief Procesa UNA muestra del ADSR — llamar desde ISR de audio a 44100Hz
 *        o desde tarea de control a la frecuencia deseada
 * @param state         Estado de la instancia (modificado internamente)
 * @param dac_output    Salida lista para enviar al DAC8564 via SPI
 */
void adsr_process_sample(adsr_state_t *state, adsr_dac_output_t *dac_output);

/**
 * @brief Convierte tiempo en ms a número de muestras (escala logarítmica)
 * @param ms    Tiempo en milisegundos
 * @return      Número de muestras equivalentes
 */
uint32_t adsr_ms_to_samples(float ms);

/**
 * @brief Escala un valor de parámetro MIDI (0–127) a tiempo en ms
 *        con la misma curva logarítmica del Digitone
 * @param midi_val  Valor 0–127
 * @return          Tiempo en ms
 */
float adsr_midi_to_ms(uint8_t midi_val);

/**
 * @brief Aplica la curva de forma seleccionada al progreso lineal [0.0, 1.0]
 * @param t         Progreso lineal normalizado (0.0 = inicio, 1.0 = fin)
 * @param curve     Tipo de curva
 * @return          Progreso curvado normalizado
 */
float adsr_apply_curve(float t, adsr_curve_t curve);

/**
 * @brief Parámetros por defecto tipo Digitone — voz percusiva
 */
extern const adsr_params_t ADSR_PRESET_PERCUSSION;

/**
 * @brief Parámetros por defecto tipo Moog — pad lento con sustain alto
 */
extern const adsr_params_t ADSR_PRESET_PAD;

/**
 * @brief Parámetros por defecto — bass lead, ataque rápido, decay medio
 */
extern const adsr_params_t ADSR_PRESET_BASS;

#endif /* ADSR_ENGINE_H */
