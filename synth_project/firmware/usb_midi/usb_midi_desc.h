/**
 * @file    usb_midi_desc.h
 * @brief   Descriptores USB MIDI Class 1.0 (Class-Compliant)
 *
 * Standard: USB MIDI Devices 1.0 (sobre USB Audio Class 1.0)
 *
 * Compatibilidad automática (sin drivers):
 *   - Windows 10/11    → Aparece en Ableton como "Synth MIDI"
 *   - macOS 12+        → Aparece en Audio MIDI Setup automáticamente
 *   - Linux (ALSA)     → /dev/snd/midiCxDx
 *   - iOS/Android      → USB Audio Class compliant
 *
 * Hardware requerido en el STM32H7:
 *   - USB OTG FS (interno, sin PHY externo) → hasta 12Mbps (FS)
 *   - Pin PA11 = USB_DM, PA12 = USB_DP
 *   - Resistor de pull-up interno (el STM32H7 lo tiene integrado via software)
 *   - Conector USB Type B estándar (o USB-C con CC1/CC2 a GND via 5.1kΩ)
 *
 * Nota sobre USB HS vs FS para MIDI:
 *   MIDI no necesita alta velocidad — el protocolo MIDI a 31.25Kbaud
 *   cabe holgadamente en USB FS (12Mbps). Usar OTG_FS interno evita
 *   necesitar el PHY externo HS (ULPI) que requiere 20+ pines extra.
 */

#ifndef USB_MIDI_DESC_H
#define USB_MIDI_DESC_H

#include <stdint.h>

/* =========================================================================
 * Identificadores USB — Personalizar según el producto
 * =========================================================================
 * VID: 0x0483 = STMicroelectronics (para prototipo/desarrollo)
 *      En producción obtener VID propio en usb.org (~5000 USD)
 *      o usar VID de ST bajo licencia para productos de bajo volumen.
 *
 * PID: Número de producto personalizable libremente dentro del VID.
 * ========================================================================= */
#define USB_MIDI_VID                0x0483U   /* ST VID — cambiar en producción */
#define USB_MIDI_PID                0x5720U   /* PID arbitrario para sintetizador */

#define USB_MIDI_MANUFACTURER_STR   "MiSynth Labs"
#define USB_MIDI_PRODUCT_STR        "HybridSynth MIDI"
#define USB_MIDI_SERIAL_STR         "HS-001"

/* =========================================================================
 * Endpoints USB MIDI
 * Endpoint 0x01: BULK OUT (Host → Device) — MIDI IN del host = comandos al synth
 * Endpoint 0x81: BULK IN  (Device → Host) — MIDI OUT del synth hacia el host
 * ========================================================================= */
#define MIDI_EP_OUT                 0x01U
#define MIDI_EP_IN                  0x81U
#define MIDI_EP_SIZE                64U       /* Max packet size USB FS BULK */

/* =========================================================================
 * Tamaños de descriptores (calculados manualmente para verificación)
 * =========================================================================
 * La estructura de descriptores USB MIDI es:
 *
 * [Device Descriptor]
 *   └─[Configuration Descriptor]
 *       ├─[Interface 0: Audio Control]   ← Requerido por USB Audio Class
 *       │   └─[AudioControl Header]
 *       └─[Interface 1: MIDI Streaming]
 *           ├─[MIDIStreaming Header]
 *           ├─[MIDI IN Jack Embedded]    ← Virtual MIDI port IN (del host)
 *           ├─[MIDI IN Jack External]    ← Representa el HW MIDI IN
 *           ├─[MIDI OUT Jack Embedded]   ← Virtual MIDI port OUT (al host)
 *           ├─[MIDI OUT Jack External]   ← Representa el HW MIDI OUT
 *           ├─[Endpoint BULK OUT]        ← Datos MIDI host→device
 *           ├─[MIDIStreaming EP OUT]      ← Asocia endpoint con IN jacks
 *           ├─[Endpoint BULK IN]         ← Datos MIDI device→host
 *           └─[MIDIStreaming EP IN]       ← Asocia endpoint con OUT jacks
 * ========================================================================= */
#define USB_MIDI_CONFIG_DESC_SIZE   101U

/* =========================================================================
 * IDs de los Jack MIDI
 * Cada jack necesita un ID único dentro del interfaz MIDI Streaming
 * ========================================================================= */
#define MIDI_JACK_IN_EMB_ID         0x01U   /* IN Jack Embedded (del software) */
#define MIDI_JACK_IN_EXT_ID         0x02U   /* IN Jack External (del hardware)  */
#define MIDI_JACK_OUT_EMB_ID        0x03U   /* OUT Jack Embedded (al software)  */
#define MIDI_JACK_OUT_EXT_ID        0x04U   /* OUT Jack External (al hardware)  */

/* =========================================================================
 * Declaración del arreglo de descriptores
 * ========================================================================= */

/** Device Descriptor — 18 bytes fijos según spec USB 2.0 */
extern const uint8_t USB_MIDI_DeviceDesc[18];

/** Configuration + Interface + Endpoint Descriptors — todo en un bloque */
extern const uint8_t USB_MIDI_ConfigDesc[USB_MIDI_CONFIG_DESC_SIZE];

/** String Descriptors */
extern const uint8_t USB_MIDI_LangIDDesc[];
extern const uint8_t USB_MIDI_ManufacturerDesc[];
extern const uint8_t USB_MIDI_ProductDesc[];
extern const uint8_t USB_MIDI_SerialDesc[];

#endif /* USB_MIDI_DESC_H */
