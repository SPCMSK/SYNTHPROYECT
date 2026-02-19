# Lista de Materiales (BOM)
## HybridSynth v1.0 — Bill of Materials Completo

> **Versión:** 1.0 — Febrero 2026
> **Moneda:** USD (precio unitario, sin envío ni impuestos)
> **Proveedores referenciados:** Mouser, DigiKey, LCSC, AliExpress (prototipo), Alfa Rivas

---

## RESUMEN ECONÓMICO

| Categoría | Costo Estimado |
|---|---|
| Semiconductores y ICs activos | $75–95 |
| Componentes pasivos | $15–25 |
| Conectores y mecánicos | $25–40 |
| PCB fabricación (5 piezas, JLCPCB) | $15–30 |
| Módulos y subcomponentes | $15–25 |
| Herramientas y consumibles (una vez) | $50–100 |
| **TOTAL ESTIMADO (1 unidad)** | **$195–315 USD** |

---

## SECCIÓN 1 — MICROCONTROLADOR Y PROCESAMIENTO

| Qty | Componente | Descripción | Referencia | Proveedor | Precio Unit. | Notas |
|---|---|---|---|---|---|---|
| 1 | **STM32H743VIT6** | MCU ARM Cortex-M7 480MHz, LQFP100 | STM32H743VIT6 | Mouser / DigiKey | $10–14 | Principal — núcleo de todo |
| 1 | Cristal 25MHz | SMD 3225, 12pF, ±10ppm | ABM3B-25.000MHZ | Mouser | $0.80 | Para PLL USB exacto |
| 2 | Condensador 12pF | COG ceramic, 0402 | — | LCSC | $0.05 c/u | Carga del cristal |
| 1 | **ST-Link V3 Mini** | Programador/debugger SWD | STLINK-V3MINIE | Mouser | $15 | Solo 1 para el proyecto |

---

## SECCIÓN 2 — CONVERSORES DIGITAL-ANALÓGICO (DAC)

| Qty | Componente | Descripción | Referencia | Proveedor | Precio Unit. | Notas |
|---|---|---|---|---|---|---|
| 1 | **PCM5242PW** | DAC audio I2S 32bit/384kHz, 112dB, TSSOP28 | PCM5242PW | Mouser | $3.50 | Salida estéreo de audio FM |
| 1 | **DAC8564IPWR** | DAC 16bit, 4 canales, SPI, TSSOP16 | DAC8564IPWR | DigiKey | $7.20 | CV para VCF/VCA/Pitch |
| 2 | Condensador 10µF | Electrolítico, 25V, SMD 1210 | — | LCSC | $0.10 c/u | Salida del PCM5242 |
| 4 | Condensador 100nF | Ceramico COG, 0402 | — | LCSC | $0.02 c/u | Desacoplo DAC |

---

## SECCIÓN 3 — FILTRO VCF ANALÓGICO

| Qty | Componente | Descripción | Referencia | Proveedor | Precio Unit. | Notas |
|---|---|---|---|---|---|---|
| 1 | **AS3320** | VCF Moog Ladder 4-polos, -24dB/Oct, DIP16 | AS3320 | Alfa Rivas / Electric Druid | $3.50–6.00 | **Comprar 3–5 de reserva** |
| 1 | Resistor tempco 3300ppm | 1kΩ, coef. temperatura +3300ppm/°C | TFPT0603L1K00FV | Mouser | $1.20 | Compensación de temperatura del VCF |
| 2 | Trimpot 10kΩ | Cermet multivueltas, 3296W | 3296W-1-103LF | Mouser | $1.50 c/u | Cal. resonancia + cal. cutoff |
| 4 | Resistor 1kΩ | 1%, 0402, metal film | — | LCSC | $0.02 c/u | Termination VCF input |
| 2 | Condensador 1nF | C0G, 100V, 0402 | — | LCSC | $0.05 c/u | Polo del filtro |
| 2 | Condensador 3.3nF | C0G, 100V, 0402 | — | LCSC | $0.05 c/u | Polo del filtro (ajustable) |
| 2 | 2N3904 | NPN transistor, SOT-23 | 2N3904 | LCSC | $0.05 c/u | Conversor V/I para ICTL del AS3320 |

---

## SECCIÓN 4 — AMPLIFICADOR DE SALIDA VCA

| Qty | Componente | Descripción | Referencia | Proveedor | Precio Unit. | Notas |
|---|---|---|---|---|---|---|
| 1 | **THAT2180LA14-U** | VCA bajo ruido dB/V, DIP8 | THAT2180LA14-U | Mouser / THAT Corp. | $4.20 | VCA principal |
| 1 | **OPA2134UA** | Op-Amp audio de bajo ruido, dual, DIP8 | OPA2134UA | Mouser | $5.50 | Buffer post-VCA |
| 2 | **NE5532P** | Op-Amp audio dual, DIP8 | NE5532P | Mouser | $0.90 c/u | Mezclador y buffers auxiliares |
| 1 | Relay OMRON G6K-2F | Relay de señal DPDT 5V | G6K-2F-5V | Mouser | $3.80 | Bypass del VCF (conmutación limpia) |
| 1 | Transistor BC547 | NPN control del relay, SOT-23 | BC547 | LCSC | $0.05 | Driver del relay |
| 1 | Diodo 1N4148 | Protección del relay | 1N4148 | LCSC | $0.04 | Flyback del relay |

---

## SECCIÓN 5 — GESTIÓN DE ENERGÍA

| Qty | Componente | Descripción | Referencia | Proveedor | Precio Unit. | Notas |
|---|---|---|---|---|---|---|
| 1 | **Conector USB-C hembra** | USB-C receptacle, 16-pin SMD | USB4085-GF-A | Mouser | $0.80 | Entrada de alimentación 5V |
| 2 | **Resistor 5.1kΩ** | 1%, 0402 | — | LCSC | $0.02 c/u | CC1 + CC2: señaliza 1.5A al cargador |
| 1 | **Polyfuse 2A** | Fusible reseteable VBUS | MF-MSMF200 | Mouser | $0.40 | Protección sobrecarga USB |
| 1 | **LT3042EMSE** | LDO ultra bajo ruido +5V_ANA, 200mA, MSOP10 | LT3042EMSE | Mouser | $5.50 | Rail analógico positivo |
| 1 | **LT3094EMSE** | LDO ultra bajo ruido -5V_ANA, 100mA, MSOP10 | LT3094EMSE | Mouser | $5.50 | Rail analógico negativo (complementario del LT3042) |
| 1 | **LT1054ISW** | Charge pump -5.5V desde +5V, SOIC8 | LT1054ISW | Mouser | $2.50 | Alimenta LT3094 (reemplaza MAX1044) |
| 1 | **ADP3335ARMZ-3.3** | LDO 3.3V/500mA, MSOP8 | ADP3335ARMZ-3.3 | Mouser | $3.20 | +3V3_DIG para STM32H7 y lógica |
| 4 | Ferrita bead 600Ω@100MHz | 0805, 1A | BLM21PG600SN1D | Mouser | $0.20 c/u | Filtro entre USB y LDOs |
| 4 | Condensador 100µF | Electrolítico, 25V, SMD | — | LCSC | $0.20 c/u | Filtrado de fuente |
| 10 | Condensador 100nF | Cerámico X5R, 0402 | — | LCSC | $0.02 c/u | Desacoplo de fuente |

---

## SECCIÓN 6 — INTERFAZ DIGITAL Y CONTROL

| Qty | Componente | Descripción | Referencia | Proveedor | Precio Unit. | Notas |
|---|---|---|---|---|---|---|
| 8 | **EC11** | Encoder rotativo con push, 20 pulsos | EC11E18244A5 | Mouser | $1.20 c/u | Control de parámetros por páginas OLED |
| 2 | **Potenciómetro B10kΩ** | 10kΩ lineal, 9mm, panel mount | RK09L1100Z3 | Mouser | $0.80 c/u | Master Volume (RV3) + Filter Cutoff (RV4) SOLO |
| 16 | **Switch mecánico Cherry MX** | Tactile/linear step switch, PCB mount | Cherry MX Brown | Mouser/Bolsa | $0.50 c/u | 16 steps del secuenciador |
| 12 | **Botón táctil/mecánico** | 6×6mm o 12×12mm función | TVAF06-A020A | Mouser | $0.20 c/u | Play/Stop/Record/Page/Bank — ~12 botones |
| 1 | **MCP23017-E/SP** | Expansor GPIO I2C 16 pines, DIP28 | MCP23017-E/SP | Mouser | $1.30 | Escaneo de switches + botones, INT_A |
| 1 | **PCA9685PW** | Driver PWM I2C 16 canales 12-bit, TSSOP28 | PCA9685PW | Mouser | $0.50 | Control de 16× LED SMD 0603 |
| 1 | **W25Q128JVSIQ** | Flash QSPI 128Mbit (16MB), SOIC8 | W25Q128JVSIQ | LCSC | $1.50 | Almacenamiento de presets |
| 1 | **OLED 128×64 SSD1306** | Display 0.96", I2C, módulo | SSD1306 module | AliExpress | $2.50 | Display principal |
| 1 | **USB Type B** | Conector USB-B chasis, traversa | USB-B-S-RA | Mouser | $0.80 | Conexión a DAW (MIDI USB) |
| 1 | **USBLC6-2SC6** | Protección ESD USB, SOT-23-6 | USBLC6-2SC6 | Mouser | $0.45 | Protección USB obligatoria |
| 1 | Polyfuse 500mA | Fusible reseteable VBUS USB-B | RXEF050 | Mouser | $0.30 | Protección cortocircuito USB MIDI |

### Conectividad MIDI DIN-5 (Opcional pero recomendado)

| Qty | Componente | Descripción | Proveedor | Precio |
|---|---|---|---|---|
| 2 | DIN-5 hembra, panel | Conector MIDI, chasis | Mouser | $1.20 c/u |
| 1 | **6N137** | Optoacoplador rápido para MIDI IN | Mouser | $0.50 |
| 2 | Resistor 220Ω | Corriente MIDI OUT | LCSC | $0.02 c/u |

### Conectividad CV/Gate (Para módulos Eurorack)

| Qty | Componente | Descripción | Proveedor | Precio |
|---|---|---|---|---|
| 4 | Jack 3.5mm TRS, panel | TS mono, chasis | LCSC | $0.60 c/u |
| 2 | Jack 6.35mm (1/4") TRS | Audio output, chasis | Mouser | $1.50 c/u |

---

## SECCIÓN 7 — INDICACIÓN Y DISPLAY

| Qty | Componente | Descripción | Referencia | Proveedor | Precio Unit. | Notas |
|---|---|---|---|---|---|---|
| 16 | **LED SMD 0603** | LED verde/ámbar monocolor, 0603 | — | LCSC | $0.04 c/u | Steps del secuenciador (controlados por PCA9685) |
| 16 | Resistor 330Ω | 1%, 0402 | — | LCSC | $0.02 c/u | Limitador de corriente para LEDs 0603 |
| 1 | LED azul 3mm | Power indicator | — | LCSC | $0.05 | Encendido del sistema |

---

## SECCIÓN 8 — PCB Y FABRICACIÓN

| Ítem | Especificación | Proveedor | Costo estimado |
|---|---|---|---|
| PCB Principal (5 piezas) | 100×100mm, 2 capas, FR4 1.6mm, HASL LF | JLCPCB | $10–15 |
| PCB Panel Frontal (2 piezas) | 300×180mm, 2 capas, ENIG | JLCPCB | $15–25 |
| Ensamblaje SMD (opcional) | Solo componentes básicos en JLCPCB Assembly | JLCPCB | +$30–50 |
| Stencil de pasta de soldar | Con la PCB principal | JLCPCB | +$8 |

---

## SECCIÓN 9 — HARDWARE MECÁNICO

| Qty | Componente | Descripción | Proveedor | Precio |
|---|---|---|---|---|
| 1 | Chasis aluminio | 300×180×60mm, doblado 2mm AL5052 | Protocase / local | $40–80 |
| 20 | Tornillo M3×8mm | Inox, cabeza Phillips | Local | $5 (pack) |
| 20 | Tuerca M3 | Inox | Local | $3 (pack) |
| 10 | Espaciador M3×10mm | Brass, para montar PCB en chasis | Local | $5 (pack) |
| - | Silicona térmica | Para LT3042 al chasis si necesita heatsink | Local | $3 |
| 1 | Panel frontal impreso | Aluminio 3mm + vinilo impreso UV | Local / online | $20–40 |
| 4 | Pies de goma | Antideslizantes para escritorio | Local | $3 |

---

## SECCIÓN 10 — CONSUMIBLES Y HERRAMIENTAS

| Ítem | Notas | Costo Est. |
|---|---|---|
| Soldadura 63/37 0.6mm | NO usar lead-free para prototipos (más difícil) | $10 |
| Flux rosin en pluma | Imprescindible para SMD | $6 |
| Pasta de soldar SMD 63/37 | Para stencil o aplicación manual | $10 |
| Limpiador IPA 99% (isopropanol) | Limpieza de flux post-soldadura | $8 |
| Hisopos / pincel fino | Limpieza PCB | $3 |
| Cinta Kapton | Aislamiento temporal durante test | $4 |
| Alambres de colores 26AWG | Puentes y modificaciones | $8 |
| Conectores JST-XH 2.54mm | Cableado interno desmontable | $10 (surtido) |
| Termoretráctil surtido | Aislamiento de joints | $5 |

---

## PROVEEDORES RECOMENDADOS

| Proveedor | País/Región | Para qué | URL |
|---|---|---|---|
| **Mouser Electronics** | USA (envío global) | Componentes de calidad garantizada | mouser.com |
| **DigiKey** | USA (envío global) | Alternativa a Mouser, mismos precios | digikey.com |
| **LCSC** | China | Componentes básicos muy económicos | lcsc.com |
| **JLCPCB** | China | Fabricación de PCBs y ensamblaje | jlcpcb.com |
| **Alfa Rivas** | Rusia/online | AS3320, AS2044, y otros chips de synth | alfasynth.com |
| **Electric Druid** | UK | Chips especializados para synth | electricdruid.net |
| **AliExpress** | China | Módulos (OLED, etc.) para prototipo | aliexpress.com |

> **ADVERTENCIA sobre AliExpress:** Solo usar para módulos y componentes no críticos de audio.
> Los semiconductores para audio (AS3320, OPA2134, DAC8564) deben comprarse en Mouser o DigiKey
> para garantizar autenticidad. El mercado gris tiene muchos ICs falsificados.
