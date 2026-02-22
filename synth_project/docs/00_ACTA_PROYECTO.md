# ACTA DE PROYECTO Y PLAN DE ACCIÓN
## HybridSynth v1.0
### Sintetizador Híbrido Digital/Analógico

---

```
Versión del documento:   1.1
Fecha de emisión:        Febrero 2026
Estado:                  ACTIVO — en desarrollo
```

### Decisiones de arquitectura tomadas (Rev 1.1)

| Decisión | Detalle |
|---|---|
| **UI estilo Digitone** | 8 encoders + display OLED + páginas, sin banco de 48 potenciómetros |
| **2 pots físicos únicamente** | Master Volume + Filter Cutoff — los únicos que justifican control táctil directo |
| **16 switches mecánicos** | Para los pasos del secuenciador — respuesta táctil real |
| **MCP23017** | Escaneo de todos los botones por I2C — 0 pines GPIO del STM32 adicionales |
| **PCA9685** | Driver LED 16 canales PWM por I2C — sustituye WS2812B |
| **USB-C 5V alimentación** | Sin PSU de 220V externa — cualquier cargador USB-C 1.5A+ |
| **Sin Fase Teensy** | Se diseña directo el PCB STM32H743 — firmware ya escrito y validado |
| **2 PCBs** | Placa principal (100×100mm) + Placa panel — tier $2 de JLCPCB |

---

## PARTE I — ACTA DE PROYECTO (Project Charter)

### 1. Descripción del Proyecto

**Nombre del proyecto:** HybridSynth v1.0

**Descripción resumida:**
Diseño, construcción y validación de un instrumento musical electrónico de escritorio
que combina síntesis digital FM de alta resolución (estilo Elektron Digitone) con
una ruta de audio analógica de componentes discretos (estilo Moog Minimoog).

El instrumento es controlable desde software de producción musical (Ableton Live, Logic Pro)
vía USB MIDI Class-Compliant, sin necesidad de drivers adicionales, y opera en 3 modos:
FM puro, Vintage Analógico y Híbrido.

---

### 2. Justificación del Proyecto

| Motivación | Descripción |
|---|---|
| Técnica | Aplicar conocimientos de electrónica civil en un sistema de hardware/firmware complejo y medible |
| Musical | Obtener un instrumento único con carácter analógico y flexibilidad digital que no existe en el mercado por precio <$600 USD |
| Formativa | Cubrir el ciclo completo: diseño de circuito → PCB → firmware embebido → integración de sistema |

---

### 3. Alcance del Proyecto

#### 3.1 Incluido en el Alcance (IN SCOPE)

- [x] Motor de síntesis FM de 4–6 operadores, 4 voces de polifonía
- [x] Motor ADSR digital con curvas configurables y múltiples destinos
- [x] Filtro analógico VCF basado en AS3320 (clon Moog Ladder), controlado por voltaje
- [x] Amplificador de salida VCA analógico de bajo ruido (THAT2180)
- [x] Interfaz USB MIDI Class-Compliant (Ableton Live compatible)
- [x] Sistema de presets: guardado/carga en memoria Flash externa (512+ presets)
- [x] Secuenciador de 16 pasos con Parameter Locks (estilo Digitone)
- [x] Panel frontal estilo Digitone: 8 encoders + OLED 128×64 + páginas de parámetros
- [x] 16 switches mecánicos para pasos del secuenciador con LED 0603 individual
- [x] 2 potenciómetros físicos: Master Volume + Filter Cutoff
- [x] ~12 botones de función: Play/Stop/Rec, Track A-D, Page, Shift, etc.
- [x] Alimentación por USB-C 5V (sin PSU de 220V) — compatible con cualquier cargador
- [x] Diseño en 2 PCBs: Placa Principal (100×100mm) + Placa Panel (conectadas por JST)
- [x] Conectividad CV/Gate estándar (1V/Oct) para integración con Eurorack
- [x] 3 modos de operación: FM Precision, Vintage Mono, Híbrido
- [x] Documentación técnica completa: esquemáticos, firmware, guía de calibración

#### 3.2 Excluido del Alcance (OUT OF SCOPE)

- [ ] ~~Producción en serie o fabricación para venta~~
- [ ] ~~Interfaz gráfica de software (VST Plugin, editor MIDI)~~ — posible Fase 2 futuro
- [ ] ~~Síntesis por muestras (sampler)~~
- [ ] ~~Conectividad inalámbrica (WiFi/Bluetooth MIDI)~~
- [ ] ~~Pantalla táctil o interfaz gráfica compleja en el dispositivo~~
- [ ] ~~Certificación CE/FCC~~ (solo uso privado/laboratorio)

---

### 4. Objetivos Medibles

| ID | Objetivo | Métrica de éxito | Método de verificación |
|---|---|---|---|
| O1 | Latencia MIDI mínima | < 3ms gate-to-audio | Osciloscopio: canal 1 = MIDI NoteOn, canal 2 = señal de audio |
| O2 | Resolución sin zipper noise | DAC 16-bit = 65,536 pasos | Movimiento de parámetro a 1Hz, verificar con analizador de espectro |
| O3 | Rango dinámico (S/N ratio) | > 80dB (objetivo 100dB) | Medición con REW + interfaz de audio 24-bit |
| O4 | Tracking VCF V/Oct | ≤ ±10 cents en 4 octavas | Comparar self-oscillation del VCF vs nota de referencia en chromatic tuner |
| O5 | Compatibilidad DAW | Reconocido sin driver en Ableton, macOS, Windows 11 | Test directo en los 3 sistemas operativos |
| O6 | Autonomía de presets | ≥ 100 presets guardados y recuperados correctamente | Test de escritura/lectura de 100 presets distintos |
| O7 | Temperatura operación | < 60°C en todos los componentes | Termómetro infrarrojo después de 2h de operación continua |

---

### 5. Entregables del Proyecto

| # | Entregable | Formato | Criterio de completitud |
|---|---|---|---|
| E1 | Esquemático PCB Principal Rev 1 | Archivo KiCad .kicad_sch | ERC sin errores críticos |
| E2 | Layout PCB Principal Rev 1 | Archivo KiCad .kicad_pcb | DRC sin errores, Gerbers exportados |
| E3 | Esquemático + Layout PCB Panel | Archivos KiCad | ERC y DRC sin errores |
| E4 | Firmware v1.0 | Repositorio Git (código C) | Compila sin warnings, pasa test básico de audio |
| E5 | PCB Principal ensamblada y probada | Hardware físico | Audio FM + VCF + MIDI funcionando |
| E6 | Panel frontal integrado | Hardware físico | 100% de controles operativos |
| E7 | Documentación técnica completa | Archivos Markdown + esquemáticos KiCad | Completa según este acta |

---

### 6. Restricciones del Proyecto

| Restricción | Detalle |
|---|---|
| **Presupuesto** | Máximo $400 USD para prototipo funcional completo (excluyendo osciloscopio si ya existe) |
| **Tiempo** | 22 semanas de desarrollo total (ver Plan de Acción) |
| **Herramientas** | Solo software gratuito/libre: KiCad, STM32CubeIDE, STM32CubeMX, GCC ARM, LTSpice, GIT |
| **Componentes** | Usar únicamente componentes disponibles en Mouser/DigiKey/LCSC con stock confirmado antes de diseñar |
| **Espacio de trabajo** | Se requiere mínimo: soldador de temperatura variable, flux, multímetro, osciloscopio básico |

---

### 7. Riesgos Identificados

| ID | Riesgo | Probabilidad | Impacto | Plan de Contingencia |
|---|---|---|---|---|
| R1 | PCB Rev 1 con errores que impidan funcionamiento | Alta | Alto | Prever 2 ciclos de revisión de PCB en el calendario; mantener 2 semanas de buffer |
| R2 | AS3320 dañado durante montaje | Media | Alto | Comprar 5 unidades; seguir lista de verificación pre-encendido |
| R3 | Incompatibilidad USB en Windows (driver) | Media | Medio | Validar descriptores con USB View antes de fabricar el PCB |
| R4 | Ruido analógico insuperable sin rediseño de PCB | Baja | Alto | Seguir estrictamente las reglas de layout EMI; posible Rev 2 de PCB si necesario |
| R5 | Desabastecimiento de componentes clave | Baja | Alto | Comprar todos los ICs críticos al inicio del proyecto |
| R6 | Error de diseño en PCB Principal Rev 1 | Alta | Medio | 2 PCBs separadas — un bug en la principal no invalida el panel; prever Rev 2 en calendario |

---

### 8. Supuestos del Proyecto

- El responsable tiene conocimientos básicos de electrónica analógica (ley de Ohm, condensadores, op-amps)
- El responsable tiene o adquirirá conocimientos básicos de C para microcontroladores
- Se cuenta con acceso a un osciloscopio de al menos 2 canales y 50MHz de ancho de banda
- Se cuenta con soldador de temperatura variable y herramientas básicas de SMD
- El acceso a internet es estable para descargar datasheets, herramientas y ejemplos
- El entorno de trabajo (calidad del ambiente eléctrico) es razonablemente libre de interferencias

---

## PARTE II — PLAN DE ACCIÓN

### 9. Cronograma Resumido

```
SEMANA   FASE         ACTIVIDAD PRINCIPAL                         ENTREGABLE
─────────────────────────────────────────────────────────────────────────────
1–2      FASE 1       Esquemático KiCad: alimentación + MCU        Bloques power+STM32 ✓
3–4      FASE 1       Esquemático KiCad: audio + analógica + UI    Esquemático completo ERC ✓
5        FASE 1       Layout PCB Principal (100×100mm)             Gerbers PCB Principal
6        FASE 1       Firmware STM32H7 base + Envío a JLCPCB       PCB en camino
7–8      FASE 2       Ensamblaje + debug PCB Principal             Audio FM por I2S ✓
9–10     FASE 3       Cadena analógica VCF + VCA                   Sonido Moog audible ✓
11–12    FASE 3       Calibración + mediciones                     VCF tracking ±10 cents ✓
13–14    FASE 4       USB MIDI completo + presets + secuenciador   16 steps P-locks ✓
15       FASE 4       Layout PCB Panel + Envío a JLCPCB            PCB Panel en camino
16–17    FASE 5       Ensamblaje panel + firmware UI               UI con encoders/OLED ✓
18       FASE 5       Test integración 2 PCBs                      Sistema completo ✓
19–20    FASE 6       PCB Rev 2 correcciones + re-fabricar         PCB definitivo
21       FASE 6       Chasis + ensamblaje final                    Instrumento integrado
22       FASE 6       Burn-in test + mediciones + demos            Entregables E1–E7 ✓
```

---

### 10. Plan de Acción Semanal Detallado — Primeras 8 Semanas

#### Semana 1 — Esquemático KiCad: Alimentación + MCU
| Día | Tarea | Duración est. |
|---|---|---|
| 1 | Instalar KiCad 8, configurar librerías, crear proyecto HybridSynth | 2h |
| 2–3 | Diseñar bloque de alimentación: USB-C + polyfuse + LT3042 + LT1054 + LT3094 + ADP3335 | 5h |
| 4 | Añadir símbolos custom (AS3320, THAT2180 no están en lib estándar de KiCad) | 3h |
| 5 | Diseñar bloque STM32H743: decoupling pin a pin, cristal 25MHz, VCAP1/VCAP2, SWD | 5h |
| 6–7 | ERC del bloque power + MCU — 0 errores antes de continuar | 3h |

#### Semana 2 — Esquemático KiCad: Audio, Analógica e Interfaz
| Día | Tarea | Duración est. |
|---|---|---|
| 1 | Diseñar USB-B (MIDI) con USBLC6-2 + polyfuse + USB-C power (CC1/CC2 5.1kΩ) | 3h |
| 2 | Diseñar PCM5242 (I2S) + DAC8564 (SPI) + buffers OPA2134 | 4h |
| 3 | Diseñar VCF AS3320: V/I converters, tempco, trimpots, atenuadores de entrada | 4h |
| 4 | Diseñar VCA THAT2180 + relay bypass + mezclador NE5532 | 3h |
| 5 | Diseñar bus I2C: PCM5242 (0x4C) + SSD1306 (0x3C) + PCA9685 (0x40) + MCP23017 (0x20) | 3h |
| 6 | Diseñar 8× encoders EC11 (pull-ups + anti-rebote) + 2× pots + MIDI DIN + CV jacks | 3h |
| 7 | ERC final completo — 0 errores — **esquemático listo** | 3h |

#### Semana 3 — Layout PCB Principal + Envío JLCPCB
| Día | Tarea | Duración est. |
|---|---|---|
| 1 | Asignar footprints, importar al PCB editor, organizar bloques (digital/analógico/power) | 3h |
| 2 | Definir planos DGND/AGND separados, punto star ground, net classes de potencia | 3h |
| 3 | Colocar ICs: STM32 primero, cristal <5mm, decoupling <0.5mm de cada pin VDD | 4h |
| 4 | Trazar potencia (USB-C → LDOs → rails +5V/+3.3V/-5V +5V_ANA), señales analógicas | 4h |
| 5 | Trazar señales digitales (SPI×2, I2S, I2C, USB differential pair 90Ω) | 4h |
| 6 | DRC sin errores, via stitching AGND, thermal vias LT3042/LT3094, revisar 3D viewer | 3h |
| 7 | Exportar Gerbers + BOM + CPL — **ENVIAR PEDIDO JLCPCB** | 2h |

#### Semana 4 — Firmware STM32H7 (durante espera del PCB ~2 semanas)
| Día | Tarea | Duración est. |
|---|---|---|
| 1 | STM32CubeMX: clock tree (480MHz PLL1, 48MHz PLL3 para USB), generar código HAL | 3h |
| 2 | CubeMX: SAI1 master TX (→ PCM5242), SPI1 (DAC8564), SPI2 (W25Q128) | 3h |
| 3 | CubeMX: USB OTG FS Device, I2C1 Fast 400kHz, USART1 31250 baud (MIDI DIN) | 3h |
| 4 | Portar `adsr_engine.c` y `usb_midi_desc.c` — compilar sin warnings | 4h |
| 5 | Implementar ISR DMA doble buffer para SAI1 (buffer en AXI SRAM, no DTCM) | 4h |
| 6 | Driver PCA9685: init + `PCA9685_SetPWM(ch, on, off)` por I2C | 3h |
| 7 | Driver MCP23017: init + lectura de 16 pines por interrupción INT + debounce | 3h |

---

### 11. Indicadores de Progreso (KPIs del Proyecto)

| KPI | Objetivo | Frecuencia de revisión |
|---|---|---|
| % de tareas completadas vs planificadas | > 80% por semana | Semanal |
| Bugs abiertos | < 5 sin resolver al pasar de fase | Al finalizar cada fase |
| Gasto acumulado vs presupuesto | < $400 total al finalizar | Mensual |
| Tiempo de compilación del firmware | < 30 segundos | Por sprint |

---

### 12. Registro de Cambios al Acta

| Versión | Fecha | Cambio | Responsable |
|---|---|---|---|
| 1.0 | Feb 2026 | Documento inicial | — |
| — | — | — | — |

---

### 13. Aprobación

```
Responsable del Proyecto: _________________________________

Firma: ________________________    Fecha: ________________

Observaciones: ________________________________________
              ________________________________________
```

---

> **Nota:** Este acta es un documento vivo. Debe actualizarse al final de cada fase
> con el estado real de los entregables, los cambios de alcance y las lecciones aprendidas.
> El historial de cambios garantiza trazabilidad completa del proyecto.
