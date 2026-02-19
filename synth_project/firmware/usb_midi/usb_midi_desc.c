/**
 * @file    usb_midi_desc.c
 * @brief   Definición de todos los descriptores USB MIDI Class 1.0
 *
 * Referencia: "Universal Serial Bus Device Class Specification for MIDI Devices"
 *             Revision 1.0, USB Implementers Forum, November 1999.
 *
 * Cada byte está comentado para facilitar modificaciones futuras.
 * Para agregar puertos MIDI virtuales adicionales (multiport),
 * agregar más pares de jack IN/OUT y actualizar USB_MIDI_CONFIG_DESC_SIZE.
 */

#include "usb_midi_desc.h"

/* =========================================================================
 * Device Descriptor
 * ========================================================================= */
const uint8_t USB_MIDI_DeviceDesc[18] = {
    0x12,           /* bLength: 18 bytes                                    */
    0x01,           /* bDescriptorType: DEVICE                              */
    0x00, 0x02,     /* bcdUSB: USB 2.0                                      */
    0x00,           /* bDeviceClass: Definido por interfaz (Audio Class)    */
    0x00,           /* bDeviceSubClass: 0                                   */
    0x00,           /* bDeviceProtocol: 0                                   */
    0x40,           /* bMaxPacketSize0: 64 bytes (EP0)                      */
    (USB_MIDI_VID & 0xFF), (USB_MIDI_VID >> 8),   /* idVendor  LSB, MSB    */
    (USB_MIDI_PID & 0xFF), (USB_MIDI_PID >> 8),   /* idProduct LSB, MSB    */
    0x00, 0x01,     /* bcdDevice: versión del dispositivo 1.00              */
    0x01,           /* iManufacturer: string index 1                        */
    0x02,           /* iProduct: string index 2                             */
    0x03,           /* iSerialNumber: string index 3                        */
    0x01,           /* bNumConfigurations: 1 configuración                  */
};

/* =========================================================================
 * Configuration Descriptor — bloque completo de 101 bytes
 *
 * Incluye en orden:
 *  1. Configuration Descriptor          (9 bytes)
 *  2. AudioControl Interface            (9 bytes)
 *  3. AudioControl Header               (9 bytes)
 *  4. MIDI Streaming Interface          (9 bytes)
 *  5. MIDIStreaming Header              (7 bytes)
 *  6. MIDI IN Jack Embedded             (6 bytes)
 *  7. MIDI IN Jack External             (6 bytes)
 *  8. MIDI OUT Jack Embedded            (9 bytes)
 *  9. MIDI OUT Jack External            (9 bytes)
 * 10. Standard Bulk OUT Endpoint        (9 bytes)
 * 11. Class-specific MS Endpoint OUT    (5 bytes)
 * 12. Standard Bulk IN Endpoint         (9 bytes)
 * 13. Class-specific MS Endpoint IN     (5 bytes)
 * ========================================================================= */
const uint8_t USB_MIDI_ConfigDesc[USB_MIDI_CONFIG_DESC_SIZE] = {

    /* ------------------------------------------------------------------ */
    /* 1. Configuration Descriptor — 9 bytes                              */
    /* ------------------------------------------------------------------ */
    0x09,           /* bLength                                              */
    0x02,           /* bDescriptorType: CONFIGURATION                       */
    USB_MIDI_CONFIG_DESC_SIZE & 0xFF,   /* wTotalLength LSB = 101          */
    USB_MIDI_CONFIG_DESC_SIZE >> 8,     /* wTotalLength MSB                */
    0x02,           /* bNumInterfaces: 2 (AudioControl + MIDIStreaming)     */
    0x01,           /* bConfigurationValue: 1                               */
    0x00,           /* iConfiguration: sin string descriptor                */
    0x80,           /* bmAttributes: bus-powered, no remote wakeup          */
    0x32,           /* bMaxPower: 100mA (0x32 × 2mA)                        */

    /* ------------------------------------------------------------------ */
    /* 2. Standard AudioControl Interface Descriptor — 9 bytes            */
    /* La clase USB Audio requiere que exista esta interfaz aunque         */
    /* no se use para audio real. Es un wrapper obligatorio del estándar.  */
    /* ------------------------------------------------------------------ */
    0x09,           /* bLength                                              */
    0x04,           /* bDescriptorType: INTERFACE                           */
    0x00,           /* bInterfaceNumber: 0                                  */
    0x00,           /* bAlternateSetting: 0                                 */
    0x00,           /* bNumEndpoints: 0 (AudioControl no usa endpoints)     */
    0x01,           /* bInterfaceClass: AUDIO                               */
    0x01,           /* bInterfaceSubClass: AUDIO_CONTROL                    */
    0x00,           /* bInterfaceProtocol: 0                                */
    0x00,           /* iInterface: sin string                               */

    /* ------------------------------------------------------------------ */
    /* 3. Class-Specific AudioControl Header — 9 bytes                    */
    /* ------------------------------------------------------------------ */
    0x09,           /* bLength                                              */
    0x24,           /* bDescriptorType: CS_INTERFACE                        */
    0x01,           /* bDescriptorSubtype: HEADER                           */
    0x00, 0x01,     /* bcdADC: Audio Device Class spec 1.0                  */
    0x09, 0x00,     /* wTotalLength: 9 bytes (solo este header)             */
    0x01,           /* bInCollection: 1 MIDI Streaming interface            */
    0x01,           /* baInterfaceNr(1): interfaz MIDI Streaming = 1        */

    /* ------------------------------------------------------------------ */
    /* 4. Standard MIDI Streaming Interface Descriptor — 9 bytes          */
    /* ------------------------------------------------------------------ */
    0x09,           /* bLength                                              */
    0x04,           /* bDescriptorType: INTERFACE                           */
    0x01,           /* bInterfaceNumber: 1                                  */
    0x00,           /* bAlternateSetting: 0                                 */
    0x02,           /* bNumEndpoints: 2 (BULK IN + BULK OUT)                */
    0x01,           /* bInterfaceClass: AUDIO                               */
    0x03,           /* bInterfaceSubClass: MIDISTREAMING                    */
    0x00,           /* bInterfaceProtocol: 0                                */
    0x00,           /* iInterface: sin string                               */

    /* ------------------------------------------------------------------ */
    /* 5. Class-Specific MIDIStreaming Header — 7 bytes                   */
    /* ------------------------------------------------------------------ */
    0x07,           /* bLength                                              */
    0x24,           /* bDescriptorType: CS_INTERFACE                        */
    0x01,           /* bDescriptorSubtype: MS_HEADER                        */
    0x00, 0x01,     /* bcdMSC: MIDIStreaming spec 1.0                        */
    /* wTotalLength: suma de todos los descriptores class-specific de      */
    /* esta interfaz: 7+6+6+9+9 = 37 bytes                                 */
    0x25, 0x00,

    /* ------------------------------------------------------------------ */
    /* 6. MIDI IN Jack Descriptor — Embedded (ID=1) — 6 bytes             */
    /* "Puerto virtual de entrada" — recibe MIDI del host (Ableton)        */
    /* ------------------------------------------------------------------ */
    0x06,           /* bLength                                              */
    0x24,           /* bDescriptorType: CS_INTERFACE                        */
    0x02,           /* bDescriptorSubtype: MIDI_IN_JACK                     */
    0x01,           /* bJackType: EMBEDDED                                  */
    MIDI_JACK_IN_EMB_ID,  /* bJackID: 1                                    */
    0x00,           /* iJack: sin string                                    */

    /* ------------------------------------------------------------------ */
    /* 7. MIDI IN Jack Descriptor — External (ID=2) — 6 bytes             */
    /* Representa el conector DIN-5 físico si lo hubiera                   */
    /* ------------------------------------------------------------------ */
    0x06,
    0x24,
    0x02,
    0x02,           /* bJackType: EXTERNAL                                  */
    MIDI_JACK_IN_EXT_ID,  /* bJackID: 2                                    */
    0x00,

    /* ------------------------------------------------------------------ */
    /* 8. MIDI OUT Jack Descriptor — Embedded (ID=3) — 9 bytes            */
    /* "Puerto virtual de salida" — envía MIDI al host (Ableton recibe)    */
    /* ------------------------------------------------------------------ */
    0x09,           /* bLength                                              */
    0x24,
    0x03,           /* bDescriptorSubtype: MIDI_OUT_JACK                    */
    0x01,           /* bJackType: EMBEDDED                                  */
    MIDI_JACK_OUT_EMB_ID, /* bJackID: 3                                    */
    0x01,           /* bNrInputPins: 1 pin de entrada                       */
    MIDI_JACK_IN_EXT_ID,  /* baSourceID(1): conectado al IN External (ID=2)*/
    0x01,           /* baSourcePin(1): pin 1                                */
    0x00,           /* iJack: sin string                                    */

    /* ------------------------------------------------------------------ */
    /* 9. MIDI OUT Jack Descriptor — External (ID=4) — 9 bytes            */
    /* Representa la salida hacia el conector DIN-5                        */
    /* ------------------------------------------------------------------ */
    0x09,
    0x24,
    0x03,
    0x02,           /* bJackType: EXTERNAL                                  */
    MIDI_JACK_OUT_EXT_ID, /* bJackID: 4                                    */
    0x01,
    MIDI_JACK_IN_EMB_ID,  /* baSourceID(1): conectado al IN Embedded (ID=1)*/
    0x01,
    0x00,

    /* ------------------------------------------------------------------ */
    /* 10. Standard Bulk OUT Endpoint — 9 bytes                            */
    /* Host → Device: Ableton → Sintetizador                               */
    /* ------------------------------------------------------------------ */
    0x09,           /* bLength                                              */
    0x05,           /* bDescriptorType: ENDPOINT                            */
    MIDI_EP_OUT,    /* bEndpointAddress: 0x01 (OUT)                         */
    0x02,           /* bmAttributes: BULK                                   */
    MIDI_EP_SIZE, 0x00, /* wMaxPacketSize: 64 bytes                         */
    0x00,           /* bInterval: 0 (BULK no usa polling interval)          */
    0x00,           /* bRefresh: 0                                          */
    0x00,           /* bSynchAddress: 0                                     */

    /* ------------------------------------------------------------------ */
    /* 11. Class-Specific MS Bulk OUT Endpoint — 5 bytes                  */
    /* Asocia el endpoint con los IN Jacks que alimenta                    */
    /* ------------------------------------------------------------------ */
    0x05,           /* bLength                                              */
    0x25,           /* bDescriptorType: CS_ENDPOINT                         */
    0x01,           /* bDescriptorSubtype: MS_GENERAL                       */
    0x01,           /* bNumEmbMIDIJack: 1 jack asociado                     */
    MIDI_JACK_IN_EMB_ID, /* baAssocJackID(1): Jack IN Embedded ID=1         */

    /* ------------------------------------------------------------------ */
    /* 12. Standard Bulk IN Endpoint — 9 bytes                             */
    /* Device → Host: Sintetizador → Ableton                               */
    /* ------------------------------------------------------------------ */
    0x09,
    0x05,
    MIDI_EP_IN,     /* bEndpointAddress: 0x81 (IN, dirección bit7=1)        */
    0x02,           /* bmAttributes: BULK                                   */
    MIDI_EP_SIZE, 0x00,
    0x00,
    0x00,
    0x00,

    /* ------------------------------------------------------------------ */
    /* 13. Class-Specific MS Bulk IN Endpoint — 5 bytes                   */
    /* ------------------------------------------------------------------ */
    0x05,
    0x25,
    0x01,
    0x01,           /* bNumEmbMIDIJack: 1                                    */
    MIDI_JACK_OUT_EMB_ID, /* baAssocJackID(1): Jack OUT Embedded ID=3        */
};

/* =========================================================================
 * String Descriptors
 * Formato: [bLength][0x03][UTF-16LE chars...]
 * ========================================================================= */

/* LangID: 0x0409 = Inglés Americano (requerido como string index 0) */
const uint8_t USB_MIDI_LangIDDesc[] = {
    0x04, 0x03,
    0x09, 0x04,
};

/* Manufacturer: "MiSynth Labs" */
const uint8_t USB_MIDI_ManufacturerDesc[] = {
    0x1A, 0x03,   /* bLength = 2 + (12 chars × 2 bytes UTF-16) = 26 = 0x1A */
    'M',0, 'i',0, 'S',0, 'y',0, 'n',0, 't',0,
    'h',0, ' ',0, 'L',0, 'a',0, 'b',0, 's',0,
};

/* Product: "HybridSynth MIDI" */
const uint8_t USB_MIDI_ProductDesc[] = {
    0x22, 0x03,   /* bLength = 2 + (16 chars × 2) = 34 = 0x22              */
    'H',0, 'y',0, 'b',0, 'r',0, 'i',0, 'd',0, 'S',0, 'y',0,
    'n',0, 't',0, 'h',0, ' ',0, 'M',0, 'I',0, 'D',0, 'I',0,
};

/* Serial: "HS-001" */
const uint8_t USB_MIDI_SerialDesc[] = {
    0x0E, 0x03,   /* bLength = 2 + (6 chars × 2) = 14 = 0x0E               */
    'H',0, 'S',0, '-',0, '0',0, '0',0, '1',0,
};
