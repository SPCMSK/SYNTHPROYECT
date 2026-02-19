# HybridSynth v1.0
### Sintetizador Híbrido Digital/Analógico

> Combinación entre un Elektron Digitone (síntesis FM digital) y un Moog (carácter analógico).
> Control 100% digital para presets y automatización. Corazón analógico para el timbre.

---

## Estructura del Proyecto

```
synth_project/
│
├── firmware/                   # Código fuente para STM32H743
│   ├── adsr/                   # Motor de envolventes ADSR digital
│   │   ├── adsr_engine.h       # API pública, tipos, estructuras
│   │   ├── adsr_engine.c       # Motor completo: curvas, DAC8564, presets
│   │   └── synth_voice.c       # Integración: voces, MIDI, callbacks
│   │
│   ├── usb_midi/               # Stack USB MIDI Class-Compliant
│   │   ├── usb_midi_desc.h     # Constantes e IDs de descriptores USB
│   │   ├── usb_midi_desc.c     # Descriptores completos USB MIDI 1.0
│   │   └── usb_midi_driver.h   # API driver: send/receive, CC map, callbacks
│   │
│   ├── fm/                     # [PENDIENTE] Motor de síntesis FM 4–6 ops
│   ├── dsp/                    # [PENDIENTE] Efectos: Reverb FDN, Delay
│   └── sequencer/              # [PENDIENTE] Secuenciador 16 pasos + P-Locks
│
├── hardware/                   # Diseño de circuito y PCB
│   ├── schematics/             # [PENDIENTE] Esquemáticos KiCad (.sch)
│   ├── pcb/                    # [PENDIENTE] Layout PCB KiCad (.kicad_pcb)
│   └── simulation/             # [PENDIENTE] Simulaciones LTSpice (.asc)
│
└── docs/                       # Documentación del proyecto
    ├── 00_ACTA_PROYECTO.md     # Acta formal + Plan de acción semana a semana
    ├── 01_PLAN_ETAPAS.md       # 6 fases, 22 semanas, checklists por semana
    ├── 02_DESGLOSE_TECNICO.md  # Arquitectura, riesgos y complicaciones
    ├── 03_LISTA_MATERIALES.md  # BOM completo con precios y proveedores
    └── 04_GUIA_INVESTIGACION.md# Qué estudiar, errores clásicos y recursos
```

---

## Estado Actual del Proyecto

| Módulo | Estado | Archivo(s) |
|---|---|---|
| ADSR Motor Digital | ✅ Implementado | `firmware/adsr/` |
| USB MIDI Descriptores | ✅ Implementado | `firmware/usb_midi/` |
| USB MIDI Driver | ✅ Cabecera lista | `firmware/usb_midi/usb_midi_driver.h` |
| Síntesis FM | 🔲 Pendiente | `firmware/fm/` |
| Efectos DSP (Reverb/Delay) | 🔲 Pendiente | `firmware/dsp/` |
| Secuenciador | 🔲 Pendiente | `firmware/sequencer/` |
| Esquemático KiCad | 🔲 Pendiente | `hardware/schematics/` |
| PCB Layout | 🔲 Pendiente | `hardware/pcb/` |
| Simulación LTSpice VCF | 🔲 Pendiente | `hardware/simulation/` |

---

## Hoja de Ruta Rápida

```
AHORA         → Fase 1: Teensy 4.1 + Audio Shield (validación de concepto)
SEMANA 5–7    → Fase 2: Esquemático + PCB en KiCad con STM32H743
SEMANA 9–12   → Fase 3: Circuito analógico VCF AS3320 + VCA THAT2180
SEMANA 13–15  → Fase 4: USB MIDI completo + presets en Flash QSPI
SEMANA 16–18  → Fase 5: Panel frontal, encoders, display OLED
SEMANA 19–22  → Fase 6: PCB Rev 2, chasis, integración y pruebas finales
```

**Duración total estimada:** 22 semanas

---

## Hardware Principal

| Componente | Función |
|---|---|
| STM32H743 @ 480MHz | MCU principal, síntesis FM, ADSR, USB MIDI |
| PCM5242 (32-bit I2S DAC) | Salida de audio digital estéreo |
| DAC8564 (16-bit SPI, 4ch) | Control de voltaje: VCF Cutoff, VCA, Pitch |
| AS3320 (clon CEM3320) | Filtro analógico Moog Ladder -24dB/Oct |
| THAT2180 | VCA analógico de bajo ruido (>120dB) |
| LT3042 × 2 | LDO ultra bajo ruido para sección analógica |
| W25Q128 (Flash QSPI 128MB) | Almacenamiento de presets |

---

## Costo Estimado

| Categoría | Estimado |
|---|---|
| Componentes activos y pasivos | $85–135 USD |
| PCB fabricación (JLCPCB) | $25–45 USD |
| Módulos y mecánicos | $50–85 USD |
| Herramientas (una vez) | $50–100 USD |
| **Total prototipo** | **~$215–365 USD** |

---

## Documentación

| Documento | Descripción |
|---|---|
| [Acta del Proyecto](docs/00_ACTA_PROYECTO.md) | Alcance, objetivos medibles, riesgos, plan día a día |
| [Plan por Etapas](docs/01_PLAN_ETAPAS.md) | 6 fases detalladas con checklists y criterios go/no-go |
| [Desglose Técnico](docs/02_DESGLOSE_TECNICO.md) | Arquitectura completa, EMI, complicaciones conocidas |
| [Lista de Materiales](docs/03_LISTA_MATERIALES.md) | BOM completo, precios, proveedores recomendados |
| [Guía de Investigación](docs/04_GUIA_INVESTIGACION.md) | Qué estudiar, errores clásicos, recursos gratuitos |

---

## Modos de Operación

| Modo | Descripción |
|---|---|
| **FM Precision** | Síntesis FM pura, bypass del VCF analógico, polifonía 4 voces |
| **Vintage Mono** | Señal FM/DCO → VCF AS3320 → VCA, comportamiento Moog monosynth |
| **Híbrido** | Capa analógica monofónica + texturas FM superiores, mezcla unificada |
