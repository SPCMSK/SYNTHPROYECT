/**
 * @file    usb_midi_driver.h
 * @brief   Driver USB MIDI sobre STM32H7 USB OTG FS
 *
 * Utiliza el middleware USB de STM32 (usbd_core) con una clase personalizada
 * que implementa la interfaz MIDI Streaming.
 *
 * Flujo de datos:
 *
 *   [Ableton Live / DAW]
 *        │  USB BULK OUT (64 bytes max)
 *        ▼
 *   usb_midi_rx_callback()     ← Llamado por el stack USB en ISR
 *        │
 *        ▼
 *   midi_rx_ring_buffer        ← Ring buffer de 256 bytes (ISR-safe)
 *        │
 *        ▼
 *   midi_parser_process()      ← Tarea FreeRTOS de prioridad media
 *        │
 *        ├─ NoteOn  → synth_note_on()
 *        ├─ NoteOff → synth_note_off()
 *        ├─ CC      → synth_set_param_from_cc()
 *        └─ PitchBend → synth_set_pitchbend()
 *
 *   [Sintetizador → DAW] (MIDI feedback, ej: arpeggiator output)
 *   usb_midi_send_event()      ← Llamado desde la tarea de secuenciador
 *        │  USB BULK IN
 *        ▼
 *   [Ableton Live / DAW]
 */

#ifndef USB_MIDI_DRIVER_H
#define USB_MIDI_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================================
 * Configuración
 * ========================================================================= */

#define MIDI_RX_BUFFER_SIZE     256U    /* Ring buffer de recepción         */
#define MIDI_TX_BUFFER_SIZE     64U     /* Tamaño del paquete de envío      */
#define MIDI_MAX_CHANNELS       16U     /* Canales MIDI 1–16                */

/* =========================================================================
 * Estructura de evento MIDI USB
 *
 * El protocolo USB MIDI encapsula cada mensaje en paquetes de 4 bytes:
 *
 *   Byte 0: Cable Number (bits 7–4) + Code Index Number CIN (bits 3–0)
 *   Byte 1: MIDI Status byte  (ej: 0x90 = NoteOn canal 0)
 *   Byte 2: MIDI Data byte 1  (ej: nota 60 = C4)
 *   Byte 3: MIDI Data byte 2  (ej: velocity 127)
 *
 * CIN define el tipo de mensaje:
 *   0x08 = Note Off        (2 data bytes)
 *   0x09 = Note On         (2 data bytes)
 *   0x0A = Aftertouch Poly (2 data bytes)
 *   0x0B = Control Change  (2 data bytes)
 *   0x0C = Program Change  (1 data byte)
 *   0x0D = Aftertouch Chan (1 data byte)
 *   0x0E = Pitch Bend      (2 data bytes)
 *   0x0F = Single Byte     (SysEx, etc.)
 * ========================================================================= */
typedef struct {
    uint8_t cin;        /* Code Index Number — tipo de mensaje              */
    uint8_t status;     /* Status byte (tipo + canal)                       */
    uint8_t data1;      /* Primer byte de dato                              */
    uint8_t data2;      /* Segundo byte de dato                             */
} midi_usb_packet_t;

/* =========================================================================
 * Tipos de mensaje MIDI parseados (post-procesado)
 * ========================================================================= */
typedef enum {
    MIDI_MSG_NONE          = 0x00,
    MIDI_MSG_NOTE_OFF      = 0x80,
    MIDI_MSG_NOTE_ON       = 0x90,
    MIDI_MSG_AFTERTOUCH    = 0xA0,
    MIDI_MSG_CONTROL       = 0xB0,
    MIDI_MSG_PROGRAM       = 0xC0,
    MIDI_MSG_CHAN_PRESSURE  = 0xD0,
    MIDI_MSG_PITCH_BEND    = 0xE0,
    MIDI_MSG_SYSEX         = 0xF0,
    MIDI_MSG_CLOCK         = 0xF8,   /* MIDI Clock — 24 pulsos/negra       */
    MIDI_MSG_START         = 0xFA,
    MIDI_MSG_STOP          = 0xFC,
} midi_msg_type_t;

typedef struct {
    midi_msg_type_t type;
    uint8_t         channel;    /* 0–15                                     */
    uint8_t         data1;      /* Nota / CC number / Program               */
    uint8_t         data2;      /* Velocity / CC value                      */
    int16_t         pitch_bend; /* -8192 → +8191 (solo para PITCH_BEND)     */
} midi_event_t;

/* =========================================================================
 * Mapa de Control Changes — asignación de CC a parámetros del synth
 *
 * Compatible con el mapa de Ableton "MIDI Learn" y con controladores
 * estándar (Compatible con MIDI CC standard GM/GS).
 * ========================================================================= */
typedef enum {
    CC_MODWHEEL         = 1,
    CC_BREATH           = 2,
    CC_VOLUME           = 7,
    CC_PAN              = 10,
    CC_EXPRESSION       = 11,
    CC_ATTACK           = 73,   /* MIDI CC 73 = standard Attack time         */
    CC_DECAY            = 75,   /* No estándar, pero convención común        */
    CC_SUSTAIN_LEVEL    = 64,   /* CC 64 normalmente es Hold pedal…          */
    CC_RELEASE          = 72,   /* MIDI CC 72 = standard Release time        */
    CC_CUTOFF           = 74,   /* MIDI CC 74 = Brightness = VCF Cutoff      */
    CC_RESONANCE        = 71,   /* MIDI CC 71 = Timbre = VCF Resonance       */
    CC_ENV_DEPTH        = 76,   /* Profundidad de env sobre VCF              */
    CC_LFO_RATE         = 77,
    CC_LFO_DEPTH        = 78,
    CC_PORTAMENTO_TIME  = 5,
    CC_ALL_NOTES_OFF    = 123,  /* MIDI panic                                */
    CC_RESET_ALL        = 121,
} midi_cc_t;

/* =========================================================================
 * API Pública
 * ========================================================================= */

/**
 * @brief Inicializa el stack USB MIDI
 *        Debe llamarse en main() antes del scheduler de FreeRTOS
 */
void usb_midi_init(void);

/**
 * @brief Envía un evento MIDI al host (DAW/Ableton)
 *        Thread-safe: puede llamarse desde cualquier tarea
 * @param event  Evento MIDI a enviar
 * @return true si el paquete fue encolado correctamente
 */
bool usb_midi_send_event(const midi_event_t *event);

/**
 * @brief Envía una nota al host (ej: output del secuenciador interno)
 */
bool usb_midi_send_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
bool usb_midi_send_note_off(uint8_t channel, uint8_t note);

/**
 * @brief Envía MIDI Clock (F8) al host — para sincronía con DAW
 *        Llamar exactamente 24 veces por pulso de negra (BPM)
 */
bool usb_midi_send_clock(void);

/**
 * @brief Procesa los paquetes recibidos del host
 *        Llamar periódicamente desde una tarea FreeRTOS (cada 1ms aprox.)
 *        Despachará callbacks de los eventos recibidos.
 */
void usb_midi_process_rx(void);

/* =========================================================================
 * Callbacks — implementar en el módulo de síntesis (synth_voice.c)
 * Son funciones débiles (weak) con implementación vacía por defecto
 * ========================================================================= */

/** Nota activada — conectar a synth_note_on() */
__attribute__((weak)) void midi_cb_note_on(uint8_t ch, uint8_t note, uint8_t vel);

/** Nota desactivada — conectar a synth_note_off() */
__attribute__((weak)) void midi_cb_note_off(uint8_t ch, uint8_t note);

/** Control Change — conectar a la UI/preset engine */
__attribute__((weak)) void midi_cb_control(uint8_t ch, uint8_t cc, uint8_t val);

/** Pitch Bend — rango -8192 a +8191 */
__attribute__((weak)) void midi_cb_pitch_bend(uint8_t ch, int16_t value);

/** Program Change — cargar preset */
__attribute__((weak)) void midi_cb_program_change(uint8_t ch, uint8_t program);

/** MIDI Clock — para sincronizar el secuenciador interno */
__attribute__((weak)) void midi_cb_clock(void);
__attribute__((weak)) void midi_cb_start(void);
__attribute__((weak)) void midi_cb_stop(void);

#endif /* USB_MIDI_DRIVER_H */
