# Guía de Diseño Esquemático
## HybridSynth v1.0 — Referencia Completa para KiCad

> **Decisión:** Se omite la Fase 1 en Teensy y se diseña directamente el PCB del STM32H743.
> **Justificación:** El código ADSR y los descriptores USB MIDI ya están escritos y validados
> lógicamente. El firmware portado al STM32H7 replica exactamente la arquitectura prevista.
> El único riesgo adicional frente al prototipo en Teensy es el clock tree y el DMA — ambos
> documentados en detalle en esta guía para evitarlos.
>
> **Alimentación:** Se usa USB-C 5V como fuente principal (sin PSU externa).
> Un resistor de 5.1kΩ en CC1 y CC2 del conector USB-C hace que cualquier cargador moderno
> entregue automáticamente 5V @ 1.5A — suficiente para todo el sistema.
> La alimentación negativa (-5V) se genera internamente con el LT1054 (charge pump desde +5V → -5.5V)
> seguido del LT3094 LDO para obtener -5V_ANA ultra-limpio.
> El conector USB-B separado sigue siendo el puerto MIDI hacia el PC/DAW.

> **Metodología:** Ir módulo por módulo. Terminar y verificar ERC de cada bloque antes de
> continuar con el siguiente. Nunca avanzar con warnings sin resolver.

---

## ÍNDICE

1. [Resumen de ICs y módulos](#1-resumen-de-ics-y-módulos)
2. [Alimentación — Top Level](#2-alimentación--top-level)
3. [STM32H743 — MCU Principal](#3-stm32h743--mcu-principal)
4. [PCM5242 — DAC de Audio I2S](#4-pcm5242--dac-de-audio-i2s)
5. [DAC8564 — DAC CV SPI](#5-dac8564--dac-cv-spi)
6. [AS3320 — VCF Analógico](#6-as3320--vcf-analógico)
7. [THAT2180 — VCA Analógico](#7-that2180--vca-analógico)
8. [USB MIDI — Conector y Protección](#8-usb-midi--conector-y-protección)
9. [Reguladores de Potencia](#9-reguladores-de-potencia)
10. [Multiplexores y Potenciómetros](#10-multiplexores-y-potenciómetros)
11. [Flash QSPI y Display OLED](#11-flash-qspi-y-display-oled)
12. [Conectividad MIDI DIN-5 y CV/Gate](#12-conectividad-midi-din-5-y-cvgate)
13. [Reglas de Layout PCB](#13-reglas-de-layout-pcb)
14. [Checklist antes de exportar Gerbers](#14-checklist-antes-de-exportar-gerbers)

---

## 1. Resumen de ICs y Módulos

| Referencia | IC | Función | Package | Datasheet |
|---|---|---|---|---|
| U1 | STM32H743VIT6 | MCU principal | LQFP100 | [st.com/stm32h743](https://www.st.com/resource/en/datasheet/stm32h743vi.pdf) |
| U2 | PCM5242PW | DAC audio I2S 32-bit | TSSOP28 | [ti.com/lit/ds/symlink/pcm5242.pdf](https://www.ti.com/lit/ds/symlink/pcm5242.pdf) |
| U3 | DAC8564IPWR | DAC CV 16-bit 4ch SPI | TSSOP16 | [ti.com/lit/ds/symlink/dac8564.pdf](https://www.ti.com/lit/ds/symlink/dac8564.pdf) |
| U4 | AS3320 | VCF Moog Ladder -24dB | DIP16 | [alfa.com/as3320](https://www.alfarzpp.lv/eng/sc/AS3320.pdf) |
| U5 | THAT2180LA14-U | VCA log dB/V | DIP8 | [thatcorp.com/2180](https://www.thatcorp.com/datashts/THAT_2180-Series_Datasheet.pdf) |
| U6 | OPA2134UA | Op-amp audio dual | DIP8 | [ti.com/opa2134](https://www.ti.com/lit/ds/symlink/opa2134.pdf) |
| U7 | NE5532P | Op-amp audio dual (2x) | DIP8 | [ti.com/ne5532](https://www.ti.com/lit/ds/symlink/ne5532.pdf) |
| U8 | LT3042EMSE | LDO +5V analógico | MSOP10 | [analog.com/lt3042](https://www.analog.com/media/en/technical-documentation/data-sheets/3042fc.pdf) |
| U9 | LT3094EMSE | LDO -5V analógico (versión negativa) | MSOP10 | [analog.com/lt3094](https://www.analog.com/media/en/technical-documentation/data-sheets/lt3094.pdf) |
| U10 | ADP3335ARMZ-3.3 | LDO 3.3V digital | MSOP8 | [analog.com/adp3335](https://www.analog.com/media/en/technical-documentation/data-sheets/ADP3335.pdf) |
| U11 | LT1054ISW | Charge pump 5V→-5.5V (inversor) | SOIC8 | [analog.com/lt1054](https://www.analog.com/media/en/technical-documentation/data-sheets/lt1054.pdf) |
| U12 | MCP23017-E/SP | Expansor GPIO I2C 16 pines | DIP28 | [microchip.com/mcp23017](https://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf) |
| U13 | W25Q128JVSIQ | Flash QSPI 128Mbit | SOIC8 | [winbond.com/w25q128jv](https://www.winbond.com/resource-files/w25q128jv%20spi%20revc%2011162016.pdf) |
| U14 | 6N137 | Optoacoplador MIDI IN | DIP8 | [broadcom.com/6n137](https://docs.broadcom.com/doc/AV02-0065EN) |
| U15 | USBLC6-2SC6 | Protección ESD USB MIDI | SOT-23-6 | [st.com/usblc6-2](https://www.st.com/resource/en/datasheet/usblc6-2.pdf) |
| U16 | PCA9685PW | LED driver 16ch PWM I2C | TSSOP28 | [nxp.com/pca9685](https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf) |
| CONN1 | USB-C receptacle (16p) | Conector USB-C power | THT/SMD | — |
| R_CC1, R_CC2 | 5.1kΩ 0402 | Negociación 1.5A con cargador | — | Resistores a GND en pines CC1 y CC2 del USB-C |

---

## 2. Alimentación — Top Level (USB-C 5V, sin PSU externa)

Este es el árbol de potencia completo. Hay que dibujarlo como el **primer bloque** del
esquemático antes de cualquier otra cosa — si la alimentación está mal, todo lo demás falla.

```
[Cargador USB-C 5V/1.5A+]   ← cualquier cargador de teléfono/laptop moderno
  │
[Conector USB-C (power only) con 5.1kΩ en CC1 y CC2]
  │
  └──► +5V_USB (1.5A garantizados, hasta 3A con cargador compatible)
          │
          ├──► [Polyfuse 2A]──► +5V_BUS  (protección cortocircuito)
          │
          ├──► [Ferrite 600Ω]──► [LT3042 #1 LDO] ──► +5V_ANA (200mA)
          │                                               └──► PCM5242 AVDD, DAC8564 AVDD
          │
          ├──► [ADP3335-3.3 LDO 5V→3.3V] ──► +3V3_DIG (500mA)
          │                                      └──► STM32H7, lógica, Flash, OLED
          │
          └──► [LT1054 charge pump 5V→-5.5V] ──► [LT3094 LDO] ──► -5V_ANA (100mA)
                                                                    └──► AS3320 VSS, op-amps VEE
```

### Presupuesto de corriente (total desde +5V_USB)

| Subsistema | Corriente típica |
|---|---|
| STM32H743 @ 480MHz + periféricos | ~130mA |
| Lógica (MCP23017, PCA9685, W25Q128, OLED) | ~30mA |
| PCM5242 + DAC8564 (rails analógicos) | ~45mA |
| AS3320 + THAT2180 + op-amps (+5V_ANA) | ~60mA |
| LT1054 overhead generando -5.5V (penalización carga bomba) | ~75mA |
| PCA9685 + 16× LED SMD 0603 (todos encendidos pleno) | ~170mA |
| **TOTAL TÍPICO** | **~515mA** |
| **PICO (todos LEDs pleno + carga máxima)** | **~620mA** |

> **Plan de corriente:**
> - Con **5.1kΩ en CC1+CC2**: el cargador USB-C entrega 5V @ **1.5A** automáticamente.
>   Cualquier cargador de teléfono/laptop fabricado desde 2020 lo soporta. Margen = 965mA libre.
> - Con cargadores PD (Macbook, GaN): pueden entregar 5V @ 3A sin negociación adicional.
> - **NO usar puertos USB-A tipo 2.0** para alimentar — solo 500mA, insuficiente.
> - Con PCA9685 + LED 0603 SMD el presupuesto de corriente es muy cómodo — sin restricciones de brillo.

### Símbolo de net en KiCad para las líneas de poder:
Usar **PWR_FLAG** en cada red de alimentación para suprimir warnings de ERC.

| Net name | Voltaje | Corriente máx. | Fuente |
|---|---|---|---|
| `+5V_USB` | +5V | 1.5–3A | Conector USB-C |
| `+5V_BUS` | +5V | 1.5A | Post-polyfuse |
| `+3V3_DIG` | +3.3V | 500mA | ADP3335 LDO |
| `+5V_ANA` | +5V | 200mA | LT3042 LDO |
| `-5V_ANA` | -5V | 100mA | LT1054 charge pump + LT3094 LDO |
| `AGND` | 0V analógico | — | Plano separado |
| `DGND` | 0V digital | — | Plano separado |

> **Regla de oro:** AGND y DGND son redes separadas en KiCad. Solo se unen físicamente
> en un punto cerca del conector USB-C de entrada (donde +5V_USB entra al PCB).
> Agregar un jumper de 0Ω en el esquemático en ese único punto de unión.

---

## 3. STM32H743 — MCU Principal

### 3.1 Datasheet / Reference Manual

- **Datasheet:** `STM32H743VI` — [descargar de st.com](https://www.st.com/resource/en/datasheet/stm32h743vi.pdf)
- **Reference Manual:** `RM0433` Rev 8 — [descargar de st.com](https://www.st.com/resource/en/reference_manual/rm0433-stm32h742-stm32h743-753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — **El documento más importante del proyecto.**
- **Errata Sheet:** `ES0392` — revisar antes de fabricar, algunos lotes tienen bugs USB.

### 3.2 Package LQFP100

El LQFP100 tiene pines numerados en sentido antihorario desde la esquina con el punto/bisel.
- Pin 1: marcado con punto en silkscreen.
- Paso de pines: 0.5mm — soldadura con pasta y stencil, **no manual sin buen equipo**.
- Thermal pad: NO tiene (a diferencia del QFN) — no preocuparse por via térmicas.

### 3.3 Decoupling — OBLIGATORIO, sin excepciones

Cada par de pines VDD/GND del STM32H743 necesita su propio condensador de decoupling.
El `RM0433` lista exactamente qué pins son de alimentación.

```
Pines de alimentación del STM32H743VIT6 (LQFP100):

Pin  11: VDDA   (ADC analógico)  → 1µF + 10nF cerámico C0G a AGND
Pin  12: VREF+  (referencia ADC) → 1µF + 10nF cerámico C0G a AGND
Pin  19: VDD    (dominio D3)     → 100nF a DGND + 4.7µF bulk
Pin  28: VCAP1  (regulador LDO interno D1) → 2.2µF a DGND (NO poner en VDD)
Pin  29: VDD    → 100nF + 4.7µF
Pin  32: VDD    → 100nF
Pin  48: VDD    → 100nF + 4.7µF
Pin  57: VCAP2  (regulador LDO interno D2) → 2.2µF a DGND
Pin  65: VDD    → 100nF
Pin  75: VDD    → 100nF + 4.7µF
Pin  76: VBAT   (RTC + backup RAM) → 100nF a DGND  (si no usas batería, a VDD)
Pin 100: VDD    → 100nF + 4.7µF (este es el pin más cercano al oscilador HSE)

VCAP1 y VCAP2: Son los condensadores del regulador interno de 1.2V del STM32H7.
NUNCA conectarlos a VDD. SOLO a GND con 2.2µF ceramico X5R.
```

### 3.4 Cristal HSE — 25MHz

```
           STM32H743
           ┌───────────────┐
           │ PH0 (OSC_IN)  │──[22Ω serie]──┬── XTAL 25MHz
           │               │               ├── 12pF a DGND
           │ PH1 (OSC_OUT) │───────────────┴── 12pF a DGND
           └───────────────┘

Componentes:
- Q1: ABM3B-25.000MHZ-10-D2Y-T (Abracon, 25MHz, ±10ppm, 12pF carga)
- C1, C2: 12pF C0G 0402
- R1: 22Ω 0402 en serie en OSC_IN (amortigua el arranque, reduce EMI)
```

**Layout crítico:** El cristal debe estar a menos de 5mm del STM32H7.
Hay que colocar plano de tierra DGND completo bajo el cristal y sus condensadores.
NUNCA trazar pistas de señal bajo el cristal.

### 3.5 Clock Tree — Configuración PLL

Esta es la configuración más importante del proyecto. En STM32CubeMX:

```
Configuración del clock tree:

HSE = 25MHz (cristal externo)

PLL1 (CPU y buses):
  DIVM1 = /5              → 25/5 = 5MHz VCO input
  MULN1 = ×192            → 5 × 192 = 960MHz VCO
  DIVP1 = /2              → 960/2 = 480MHz → SYSCLK (CPU, AXI, AHB)
  DIVQ1 = /4              → 960/4 = 240MHz (disponible para SPI/QSPI)
  DIVR1 = /2              → 960/2 = 480MHz (disponible)

PLL3 (USB y SAI — audio):
  DIVM3 = /5              → 25/5 = 5MHz VCO input
  MULN3 = ×192            → 5 × 192 = 960MHz VCO
  DIVP3 = /20             → 960/20 = 48MHz exacto → USB OTG FS ✓
  DIVQ3 = /20             → 960/20 = 48MHz → SAI1 kernel clock
  DIVR3 = /10             → 960/10 = 96MHz (spare)

Con SAI1 kernel = 48MHz y MCLK_div = 512:
  I2S MCLK = 48MHz / 1 = 48MHz → no se usa direct, SAI genera BCK y LRCK
  Para fs = 44100Hz:  48MHz / (44100 × 512/256) ≈ configurar en STM32CubeMX
  → Usar 48000Hz como frecuencia de audio (más limpio con este PLL)

VERDAD IMPORTANTE: 48kHz es preferible a 44.1kHz con este oscilador.
Para 44.1kHz exacto se necesita cristal de 49.152MHz — documentado por ST.
Usar 48kHz no tiene ninguna penalización sonic en este proyecto.
```

### 3.6 Asignación de Pines — Tabla Completa

| Pin STM32H743 | Función | Periférico | Net en KiCad | Notas |
|---|---|---|---|---|
| PA11 | USB_DM | OTG_FS | USB_DM | Connected via USBLC6-2 |
| PA12 | USB_DP | OTG_FS | USB_DP | Connected via USBLC6-2 |
| PA0 | ADC1_IN0 | ADC1 CH0 | POT_VOLUME | Master Volume (RV3 wiper) |
| PA1 | ADC1_IN1 | ADC1 CH1 | POT_CUTOFF | Filter Cutoff (RV4 wiper) |
| PB10 | SAI2_SCK_A | SAI2 (opciones) | — | Alternativa — ver nota SAI |
| PE2 | SAI1_MCLK_A | SAI1 | I2S_MCLK | MCLK para PCM5242 |
| PE3 | SAI1_SD_B | SAI1 | I2S_SD | Data → PCM5242 DIN |
| PE4 | SAI1_FS_A | SAI1 | I2S_LRCK | Word clock → PCM5242 LRCK |
| PE5 | SAI1_SCK_A | SAI1 | I2S_BCK | Bit clock → PCM5242 BCK |
| PA5 | SPI1_SCK | SPI1 | DAC_SCK | DAC8564 SCLK |
| PA6 | SPI1_MISO | SPI1 | DAC_MISO | DAC8564 SDO (readback) |
| PA7 | SPI1_MOSI | SPI1 | DAC_MOSI | DAC8564 SDI |
| PB6 | SPI1_NSS (via GPIO) | GPIO OUT | DAC_CSn | CS activo LOW |
| PB7 | GPIO OUT | GPIO OUT | DAC_LDAC | Pulso LOW para actualizar los 4ch |
| PB4 | SPI2_MISO | SPI2 | FLASH_MISO | W25Q128 DO |
| PB5 | SPI2_MOSI | SPI2 | FLASH_MOSI | W25Q128 DI |
| PB13 | SPI2_SCK | SPI2 | FLASH_SCK | W25Q128 CLK |
| PB14 | GPIO OUT | GPIO OUT | FLASH_CSn | Activo LOW |
| PB8 | I2C1_SCL | I2C1 | I2C_SCL | OLED + PCM5242 + PCA9685 (bus compartido) |
| PB9 | I2C1_SDA | I2C1 | I2C_SDA | OLED + PCM5242 + PCA9685 (bus compartido) |
| PB3 | TIM2_CH2 (o GPIO) | ENCODER | ENC1_A | Encoder rotativo 1 |
| PA15 | TIM2_CH1 | ENCODER | ENC1_B | Encoder rotativo 1 |
| PC6..PC12 | GPIO IN | GPIO | ENC2..ENC8 | Encoders 2–8 (TIM3/TIM4) |
| PC0 | GPIO IN (EXTI) | GPIO | MCP23017_INT | Interrupción MCP23017 INT_A (activo LOW) |
| UART1_TX | PA9 | USART1 | MIDI_TX | MIDI DIN OUT (via 220Ω) |
| UART1_RX | PA10 | USART1 | MIDI_RX | MIDI DIN IN (via 6N137) |

> **Nota SAI:** Usar SAI1 en modo master transmitter. El PCM5242 actúa como slave.
> STM32H7 genera BCK, LRCK y MCLK. Asegurarse de habilitar MCLK en CubeMX
> (es opcional, pero el PCM5242 lo necesita si no tiene oscilador propio).

### 3.7 Conector SWD — Programación/Debug

```
SWD 10 pines (ARM estándar JTAG/SWD):
Pin 1: +3V3_DIG  (VCC)
Pin 2: SWDIO (PA13)
Pin 3: GND
Pin 4: SWDCK (PA14)
Pin 5: GND
Pin 6: SWO (PB3) — opcional
Pin 7: KEY (no conectar)
Pin 8: NC
Pin 9: GND
Pin 10: nRESET

Conector recomendado: Samtec FTSH-105-01-L-DV-K (10p, 1.27mm pitch)
Compatible con ST-Link V3 Mini y cualquier J-Link.
```

---

## 4. PCM5242 — DAC de Audio I2S

### 4.1 Datasheet
[TI PCM5242 — ti.com/lit/ds/symlink/pcm5242.pdf](https://www.ti.com/lit/ds/symlink/pcm5242.pdf)

### 4.2 Descripción
- DAC estéreo 32-bit / 384kHz, SNR 112dB, THD+N < -93dB
- Interfaz digital: I2S esclavo (recibe BCK, LRCK, DATA del STM32)
- Interfaz de configuración: I2C (dirección 0x4C o 0x4D según ADDR0)
- Salida analógica: diferencial y single-ended, voltaje máximo ±2.1V rms
- Alimentación: DVDD = 3.3V, AVDD = 5V (analógico separado)
- Tiene oscilador RC interno — pero usar MCLK externo del STM32 para sincronía perfecta

### 4.3 Conexiones Esquemáticas

```
STM32H743 SAI1          PCM5242 (TSSOP28)
──────────────          ─────────────────
PE5 (I2S_BCK)  ──────► Pin 3  (BCK)   − Bit clock
PE4 (I2S_LRCK) ──────► Pin 4  (LRCK)  − Word clock (fs = 48kHz)
PE3 (I2S_SD)   ──────► Pin 5  (DIN)   − Audio data
PE2 (I2S_MCLK) ──────► Pin 2  (SCK)   − System clock (MCLK = 512×fs = 24.576MHz)

PB8 (I2C_SCL)  ──[4.7kΩ pull-up a +3V3]── Pin 17 (SCK/I2C) − Config I2C
PB9 (I2C_SDA)  ──[4.7kΩ pull-up a +3V3]── Pin 16 (SDA/I2C) − Config I2C

Pin 22 (ADDR0)  ──── DGND              → Dirección I2C = 0x4C
Pin 18 (DVDD)   ──── +3V3_DIG + 100nF a DGND + 4.7µF
Pin 19 (DGND)   ──── DGND
Pin 1  (AVDD)   ──── +5V_ANA + 100nF a AGND + 4.7µF
Pin 21 (AGND)   ──── AGND (IMPORTANTE: pin agnd va al plano analógico)
Pin 23 (CPVDD)  ──── +5V_ANA + 10µF + 100nF (charge pump supply)
Pin 24 (VCOM)   ──── 10µF a AGND   ← condensador de desacoplo del VCOM interno
Pin 27 (OUTP_L) ──[10µF AC-coupling]──► OPA2134 IN+ (canal L)
Pin 28 (OUTN_L) ──[10µF AC-coupling]──► OPA2134 IN- (canal L)
Pin 25 (OUTP_R) ──[10µF AC-coupling]──► AS3320 IN o mezcla
Pin 26 (OUTN_R) ── AGND via 100Ω    ← si se usa single-ended, terminar aquí

```

> **Nota:** El PCM5242 tiene salida diferencial. Para el VCF (mono), usar solo
> el canal L diferencial a través de un op-amp convertidor diferencial-a-single-ended.
> El canal R puede usarse para mezcla dry/wet o salida estéreo directa.

### 4.4 Inicialización I2C obligatoria

El PCM5242 arranca en modo de clock automático, pero hay que configurarle la frecuencia
exacta de oversampling via I2C. Con MCLK = 24.576MHz y fs = 48kHz: OSC = 512×fs.

```c
// Registro 37 (0x25): I2S data format → 32-bit I2S
PCM5242_WriteReg(0x25, 0x03);  // 32-bit, I2S format, MSB first
// Registro 19 (0x13): Clock source → MCLK (no requiere PLL interno)
PCM5242_WriteReg(0x13, 0x10);  // MCLK divider reference = MCLK input
// Registro 2 (0x02): Power state → run
PCM5242_WriteReg(0x02, 0x00);  // release standby, normal operation
```

---

## 5. DAC8564 — DAC CV SPI

### 5.1 Datasheet
[TI DAC8564 — ti.com/lit/ds/symlink/dac8564.pdf](https://www.ti.com/lit/ds/symlink/dac8564.pdf)

### 5.2 Descripción
- DAC de 16 bits, 4 canales independientes, interfaz SPI (hasta 50MHz)
- Salida de voltaje: 0V → VREF (pin VREF + = +5V → salida 0 a 5V)
- No necesita amplificador de salida — rail-to-rail interno
- SPI: 24 bits por transferencia (8 bits control + 16 bits dato)
- Necesita pulso en LDAC_n para actualizar todos los canales simultáneamente

### 5.3 Conexiones Esquemáticas

```
STM32H743 SPI1          DAC8564 (TSSOP16)
──────────────          ─────────────────
PA5 (SPI_SCK)  ──────► Pin 11 (SCLK)   − Hasta 50MHz
PA7 (SPI_MOSI) ──────► Pin 10 (SDI)    − Datos MSB-first
PA6 (SPI_MISO) ◄────── Pin 9  (SDO)    − Readback (opcional)
PB6 (CS_n)     ──────► Pin 12 (~CS)    − Active LOW, pulso por transferencia
PB7 (LDAC_n)   ──────► Pin 13 (~LDAC)  − Pulso LOW para update simultáneo de 4ch
                                          (si no se pulsa: los datos quedan en input reg)

Pin 1  (VREF+)  ──── +5V_ANA (referencia = rango de salida 0-5V)
Pin 2  (VREF_b) ──── +5V_ANA (boosted ref — conectar a VREF+ en modo normal)
Pin 15 (AVDD)   ──── +5V_ANA + 100nF a AGND + 4.7µF
Pin 14 (PVDD)   ──── +5V_DIG + 100nF a DGND
Pin 16 (GND)    ──── AGND

Salidas (buffer con OPA2134):
Pin 3  (VOUTA)  ──[100Ω]──► OPA2134 buffer ──► VCF_CV (jack + AS3320 ICTL)
Pin 4  (VOUTB)  ──[100Ω]──► OPA2134 buffer ──► VCA_CV (THAT2180 IN-)
Pin 5  (VOUTC)  ──[100Ω]──► NE5532  buffer ──► PITCH_CV (jack CV/Gate)
Pin 6  (VOUTD)  ──[100Ω]──► NE5532  buffer ──► AUX_CV   (jack CV/Gate)
```

> **El resistor de 100Ω** entre el pin de salida del DAC8564 y la entrada del buffer
> op-amp evita oscilaciones si la capacitancia del trace es alta (>100pF).

### 5.4 Formato SPI — 24 bits

```
Bits [23:20]: Control
  0001 → Write to input register of channel A
  0011 → Write to input register of channel B
  0101 → Write to input register of channel C
  0111 → Write to input register of channel D
  1001..1111 → Write all + update (depende del bit 20)

Bits [19:4]: Dato DAC (16-bit, MSB first)
Bits [3:0]:  Irrelevante (se pueden enviar como 0)

Secuencia para actualizar VCF:
  1. CS_n LOW
  2. Enviar: 0x10 | (valor>>12 & 0x0F) [primer byte]
             (valor>>4) & 0xFF           [segundo byte]
             (valor<<4) & 0xF0          [tercer byte]
  3. CS_n HIGH
  4. LDAC_n → LOW (>10ns) → HIGH  ← este pulso activa los 4 canales
```

---

## 6. AS3320 — VCF Analógico

### 6.1 Datasheet
[Alfa Rivas AS3320 — alfarzpp.lv/eng/sc/AS3320.pdf](https://www.alfarzpp.lv/eng/sc/AS3320.pdf)

### 6.2 Descripción
- Clon del CEM3320 original (filtro del Moog Minimoog)
- Filtro de escalera de transistores, 4 polos, -24dB/Ocatva
- Control de cutoff por corriente (pin ICTL)
- Control de resonancia por corriente (pin IRES)
- Alimentación: ±5V o ±12V (usaremos ±5V — menor ruido)
- Package DIP16 (NO existe en SMD — hay que poner socket DIP16 en el PCB)

### 6.3 Esquemático Completo

```
                    AS3320 (DIP16)
                    ┌─────────────────────┐
Audio IN+ ─[1kΩ]──►│ Pin 2 (IN+)         │
Audio IN- ─[1kΩ]──►│ Pin 3 (IN-)         │   Pin 14 (LP OUT 4-polo) ──[10kΩ]──► OPA2134 IN+
                    │                     │   Pin 13 (LP OUT 2-polo) ← NC o para BP mix
+5V_ANA ──[100nF]──►│ Pin 1 (VCC +5V)    │
-5V_ANA ──[100nF]──►│ Pin 16 (VSS -5V)   │
AGND            ──►│ Pin 8 (GND)         │
                    │                     │
ICTL_CV ──►[2N3904]│ Pin 13 (ICTL)       │   Conversor V/I para cutoff
IRES_CV ──►[2N3904]│ Pin 9 (IRES)        │   Conversor V/I para resonancia
                    └─────────────────────┘

VCF CV (de DAC8564 VOUTA, buffereado) → 0–5V
  │
  ├── [TFPT0603L1K00FV 1kΩ tempco +3300ppm/°C]
  │                                     ← OBLIGATORIO para compensación de temperatura
  └── Base de Q1 (2N3904 SOT-23)
         Colector ──► Pin 13 (ICTL) del AS3320
         Emisor   ──[470Ω]──► -5V_ANA
         (El NPN convierte voltaje en corriente para controlar el OTA del AS3320)

Resonance CV (de DAC8564 VOUTB, o un potenciómetro) → 0–4V
  │
  └── Base de Q2 (2N3904 SOT-23)
         Colector ──► Pin 9 (IRES) del AS3320 + Trimpot 10kΩ Serie
         Emisor   ──[470Ω]──► -5V_ANA

Nota sobre el trimpot de resonancia:
  - 3296W-1-103LF (10kΩ multivuela, Bourns)
  - Conectado en serie en el colector de Q2 antes del pin IRES
  - Limita la corriente máxima de resonancia → previene autooscilación incontrolada
  - Ajustable desde el exterior del chasis con destornillador (o con tornillo accesible)
```

### 6.4 Señal de Audio de Entrada

El PCM5242 produce hasta ±2.1V rms. El AS3320 acepta señales de entrada de ±0.5V máx
(clipping interno). Es obligatorio atenuar la señal antes del filtro:

```
PCM5242 OUTP_L ──[10kΩ]──┬── AS3320 IN+  ← divisor 10:1 → ±0.21V rms → dentro del rango
              AGND ──[1kΩ]─┘             ← (10k + 1k = 10:1 aproximado)
```

---

## 7. THAT2180 — VCA Analógico

### 7.1 Datasheet
[THAT Corporation 2180 — thatcorp.com/datashts/THAT_2180-Series_Datasheet.pdf](https://www.thatcorp.com/datashts/THAT_2180-Series_Datasheet.pdf)

### 7.2 Descripción
- VCA de bajo ruido con control logarítmico: 20mV/dB (o -6.6mV/dB según el modo)
- Rango dinámico: -80dB a +20dB de ganancia
- Alimentación: ±15V máx, usar ±5V (perfectamente funcional)
- Package DIP8

### 7.3 Conexiones Básicas (Modo Inversión de Señal)

```
                    THAT2180 (DIP8)
                    ┌──────────────┐
Audio IN ─[10kΩ]──►│ Pin 1 (IN-)  │
                    │              │   Pin 5 (OUT-) ──[10kΩ FB]──► Pin 1 (IN-)
+5V_ANA ──────────►│ Pin 8 (V+)   │                     └──────────────────────►  OPA2134 IN+
-5V_ANA ──────────►│ Pin 4 (V-)   │
AGND    ──────────►│ Pin 2 (GND)  │
                    │              │
VCA_CV (DAC8564 VOUTB, 0-5V)      │
  → Convertir a escala dB antes:   │
    level_dB = 20*log10(cv/5.0)   │
    control_V = level_dB * -0.02V │
                    │              │
control_V ─────────►│ Pin 3 (Ec)   │ ← Pin de control logarítmico
                    └──────────────┘

Condensador de desacoplo de entrada: 10µF electrolítico en serie en audio IN
Condensador de desacoplo de salida: 10µF electrolítico en serie después del OPA2134
```

> **La conversión a dB la hace el firmware** en `adsr_engine.c` antes de enviar
> el valor al DAC8564 canal B. Ver la función que ya está escrita en el código fuente.

---

## 8. USB MIDI — Conector y Protección

### 8.1 Esquemático

```
                    USBLC6-2SC6 (SOT-23-6)
                    ┌─────────────────────────┐
USB Tipo B Pin 2 ──►│ I/O1 (D-)               │──► PA11 (USB_DM) del STM32
USB Tipo B Pin 3 ──►│ I/O2 (D+)               │──► PA12 (USB_DP) del STM32
USB Tipo B Pin 1 ──►│ VCC (VBUS protección)   │
USB Tipo B Pin 4 ──►│ GND                     │──► DGND
                    └─────────────────────────┘

Polyfuse RXEF050 (500mA) en VBUS:
  USB Tipo B Pin 1 ──[RXEF050]──► +5V_USB ──► STM32H7 VBUS detect (PA9/OTG_FS)
  (NO necesitas +5V desde USB para alimentar el sintetizador — solo para detección)
  Si detectas VBUS en PA9, el STM32 puede saber si está conectado a PC.

Conectar en el conector USB-B:
  Pin 1 (VBUS) ──[Polyfuse]──► VBUS_DETECT ──[100kΩ]──► DGND (divisor para leer ADC)
  Pin 2 (D-)   ──────────────► USBLC6-2 I/O1
  Pin 3 (D+)   ──────────────► USBLC6-2 I/O2
  Pin 4 (GND)  ──────────────► DGND
  Shell        ──────────────► Chasis/tierra de chasis (PE - Protective Earth)
```

### 8.2 Configuración en CubeMX

- Periférico: `USB_OTG_FS`
- Modo: `Device_Only`
- Speed: `Full Speed (12Mbps)`
- VBUS sensing: `Enable` (lee en PA9)
- SOF output: `Disable`
- Pull-up interno D+: `Enable` (el STM32H7 lo tiene integrado)

> **Documentación ya escrita:** Los descriptores USB completos están en
> `firmware/usb_midi/usb_midi_desc.c` y `usb_midi_desc.h`.
> Solo hay que integrarlos con las callbacks de la HAL de CubeMX.

---

## 9. Reguladores de Potencia

### 9.1 Conector USB-C + Resistores CC (Negociación 1.5A)

```
Conector USB-C (16 pines, receptacle):
  Pin A5/B5 (CC1) ──[5.1kΩ a DGND]  ← señaliza al cargador: "soy dispositivo, dame 1.5A"
  Pin A6/B6 (CC2) ──[5.1kΩ a DGND]  ← obligatorio en AMBOS CC1 y CC2
  Pin A4/B4/A9/B9 (VBUS) ──[Polyfuse 2A]──► +5V_BUS
  Pin A1/B1/A12/B12 (GND) ──► DGND
  Pines de datos (A2,A3,B2,B3): NO CONECTAR (este conector es solo power)
  Shell ──► Chasis (Protective Earth)

NOTA: Los valores 5.1kΩ en CC son exactamente los definidos por la spec USB-C
  para indicar "Default USB Type-C Current @ 1.5A" al cargador host.
  No hace falta ningún IC de PD — solo dos resistores.
  Si el cargador es compatible USB-C PD, él mismo detecta los resistores
  y establece 5V/1.5A sin negociación DFP/UFP activa.
```

**Conector recomendado:** HRO TYPE-C-31-M-12 (SMD, 16 pines, muy común en LCSC)
Alternativa THT más robusta: Amphenol 12401610E4#2A

### 9.2 LT3042 — LDO Ultra Bajo Ruido (+5V Analógico)

**Datasheet:** [analog.com/lt3042](https://www.analog.com/media/en/technical-documentation/data-sheets/3042fc.pdf)

```
+5V_BUS ──[Ferrite 600Ω@100MHz, 1A]──► LT3042 VIN
                                        LT3042 VOUT ──► +5V_ANA
                                        LT3042 SET  ──[100kΩ a AGND]  ← fija Vout a 5V
                                        LT3042 ILIM ──[360kΩ a AGND]  ← limita a ~200mA
                                        LT3042 EN   ──► +5V_BUS (siempre ON)
                                        LT3042 GND  ──► AGND
                                        C_IN:  10µF X5R 0805 + 100nF C0G a AGND
                                        C_OUT: 10µF X5R 0805 + 100nF C0G a AGND
                                        C_SET: 10nF a AGND (en el pin SET) ← reduce ruido
```

### 9.3 ADP3335-3.3 — LDO 5V → 3.3V Digital

**Datasheet:** [analog.com/adp3335](https://www.analog.com/media/en/technical-documentation/data-sheets/ADP3335.pdf)

```
+5V_BUS ──► ADP3335 VIN
            ADP3335 VOUT ──► +3V3_DIG
            ADP3335 EN   ──► +5V_BUS (siempre ON)
            ADP3335 GND  ──► DGND
            C_IN:  10µF X5R 0805 + 100nF C0G a DGND
            C_OUT: 10µF X5R 0805 + 100nF C0G a DGND
            (la versión -3.3 es fija — no necesita divisor resistivo externo)
```

### 9.4 LT1054 — Charge Pump 5V → -5.5V

**Datasheet:** [ti.com/lt1054](https://www.ti.com/lit/ds/symlink/lt1054.pdf)

```
+5V_BUS ──► LT1054 VIN (pin 8)
            LT1054 VOUT (pin 5) ──► -5.5V crudo (estable bajo carga gracias a oscilador interno)
            C1+ (pin 2), C1- (pin 1): condensador de volante 10µF electrolítico
            C_OUT (externo): 10µF electrolítico en VOUT a GND
            ↓
            -5.5V crudo ──► LT3094 VIN
            LT3094 VOUT ──► -5V_ANA (estabilizado, ultra-limpio)

Ventaja vs MAX1044: El LT1054 trabaja internamente a ~100kHz (vs las 10kHz del MAX1044)
y entrega -5.5V robustos desde +5V con hasta 100mA de salida.
El LT3094 filtra el ripple de 100kHz con PSRR > 40dB a esa frecuencia.
Headroom = 500mV (5.5V − 5.0V) — suficiente para que LT3094 regule -5V limpio.

NOTA: El LT1054 tiene el mismo footprint DIP8/SOIC8 que el MAX1044 (pinout compatible).
Cambio de BOM trivial: solo reemplazar U11 en el esquemático.
```

### 9.5 LT3094 — LDO Ultra Bajo Ruido (-5V Analógico)

**Datasheet:** [analog.com/lt3094](https://www.analog.com/media/en/technical-documentation/data-sheets/lt3094.pdf)

```
LT3094 es el complemento negativo del LT3042 — mismo package MSOP10, pinout ESPEJADO.

-5.5V (de LT1054) ──► LT3094 IN
                       LT3094 OUT ──► -5V_ANA
                       LT3094 SET ──[100kΩ a AGND] ← fija Vout a -5V
                       LT3094 GND ──► AGND
                       C_IN:  10µF X5R a AGND
                       C_OUT: 10µF X5R a AGND
                       C_SET: 10nF a AGND

ATENCIÓN pinout: en el LT3094 los pines VIN y VOUT están intercambiados
respecto al LT3042. Verificar el datasheet antes de colocar el footprint.
Los pines son: OUT, GND×4, IN, SET, ILIM, PG, EN (izquierda a derecha en MSOP10)
```

### Resumen: ICs eliminados vs. arquitectura original

| Eliminado | Reemplazado por | Motivo |
|---|---|---|
| Mean Well IRM-30-12 (PSU 220V) | Conector USB-C + R 5.1kΩ | Alimentación desde cargador externo |
| TPS62150 (Buck 12V→5V) | Nada | Ya no hay 12V |
| LT3042 #2 (para rail negativo — error) | **LT3094EMSE** | El LT3042 no funciona con voltajes negativos |
| MAX1044 (generaba -12V) | **LT1054** (genera -5.5V desde +5V) | Mayor headroom para el LT3094 |

---

## 10. Interfaz de Usuario — Estilo Digitone

> **Decisión de diseño:** Se eliminaron 48 potenciómetros y 3× CD74HC4067 muxes.
> La UI sigue el paradigma Digitone: 8 encoders + páginas OLED + 16 mechanical switches + ~12 botones.
> Solo 2 potenciómetros físicos permanecen: Master Volume y Filter Cutoff.

### 10.1 MCP23017 — Expansor GPIO I2C (Botones y Switches)

**Datasheet:** [microchip.com/mcp23017](https://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf)

El MCP23017 proporciona 16 GPIO adicionales por I2C. Se usa para escanear:
- 16× mechanical step switches (Cherry MX style, para el secuenciador)
- ~12× botones de función (Play/Stop/Record/Page/Bank/etc. — un banco de 16 GPIOs total)

```
STM32H743           MCP23017 (DIP28, I2C 0x20)
──────────           ───────────────────────────
PB8 (I2C_SCL) ────► Pin 12 (SCL)
PB9 (I2C_SDA) ────► Pin 13 (SDA)
PC0 (EXTI) ◄────── Pin 20 (INT_A) ← interrupción activa LOW cuando cambia estado
                   Pin 18 (RESET_n) ──► +3V3_DIG via 10kΩ (sin reset externo)
                   Pin 9  (VDD) ──► +3V3_DIG + 100nF a DGND
                   Pin 10 (VSS) ──► DGND
                   Pin 15..17 (A0/A1/A2) ──► DGND → dirección 0x20

                   Port A (GPA0..GPA7): 8× step switches (steps 1–8)
                   Port B (GPB0..GPB7): 8× step switches (steps 9–16)
                                        o botones de función según layout
```

**Resistores pull-up para cada pin de switch:**
```
  GPA0..GPA7, GPB0..GPB7:
    IODIR = 1 (input)
    GPPU  = 1 (pull-up interno 100kΩ habilitado) ← evita resistores externos
    Pressed = LOW (switch cierra a GND)
    Released = HIGH (pull-up interno)
```

**Flujo de interrupción:**
```c
// En ISR de EXTI (PC0 → INT_A del MCP23017):
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == MCP23017_INT_Pin) {
        // Leer MCP23017_INTCAP (captura el estado en el momento del cambio)
        uint16_t captured = MCP23017_ReadINTCAP();
        // Leer también INTFLAG para saber qué pin disparó
        uint8_t  flag     = MCP23017_ReadINTF();
        ui_handle_button_event(captured, flag);
    }
}
```

### 10.2 Encoders Rotativos EC11 (8 unidades)

8× encoders EC11 con push. Conexión con filtro anti-rebote hardware:

```
EC11 encoder (por cada uno de 8):
  Pin A ──[10kΩ pull-up a +3V3_DIG]── TIMx_CHy (quadrature input) + [100nF a GND]
  Pin B ──[10kΩ pull-up a +3V3_DIG]── TIMx_CHy (quadrature input) + [100nF a GND]
  Pin C (push) ──[10kΩ pull-up a +3V3_DIG]── GPIO input + [100nF a GND]
  COM ──► DGND

Asignación de timers:
  Encoder 1: TIM2_CH1/CH2 (PA15/PB3)
  Encoder 2: TIM3_CH1/CH2 (PC6/PC7)
  Encoder 3: TIM4_CH1/CH2 (PD12/PD13)
  Encoder 4: TIM5_CH1/CH2 (PA0/PA1) ← ver conflicto con ADC si se usa
  Encoders 5–8: TIM8 y TIM15 (revisar disponibilidad en CubeMX)
  Push de cada encoder: GPIO IN con EXTI o polling
```

> **Nota:** STM32H743 tiene timers de encoder en modo quadrature (TIM_ENCODER_MODE_TI12).
> Contar 4 flancos por muesca del EC11 → 80 conteos por vuelta para un EC11 de 20 pulsos.
> Usar `__HAL_TIM_GET_COUNTER()` en timer de 16 bits y calcular delta respecto al valor anterior.

### 10.3 Potenciómetros (solo 2 físicos)

Solo 2 potenciómetros quedan en el diseño: **Master Volume (RV3)** y **Filter Cutoff (RV4)**.

```
RV3 — Master Volume (10kΩ lineal):
  Pin 1 (CCW) ──► AGND
  Pin 2 (wiper) ──► PA0 (ADC1_IN0) — net: POT_VOLUME
  Pin 3 (CW)  ──► +3V3_DIG (máx 3.3V para ADC del STM32)

RV4 — Filter Cutoff (10kΩ lineal):
  Pin 1 (CCW) ──► AGND
  Pin 2 (wiper) ──► PA1 (ADC1_IN1) — net: POT_CUTOFF
  Pin 3 (CW)  ──► +3V3_DIG

CRÍTICO: NO usar +5V_ANA en el extremo CW — el ADC del STM32H743 es 3.3V máximo.
Condensadores de filtro de ADC: 10nF entre wiper y AGND (reduce ruido de aliasing).
```

---

## 11. Flash QSPI y Display OLED

### 11.1 W25Q128JVSIQ — Flash 16MB SPI

```
STM32H743 SPI2         W25Q128 (SOIC8)
───────────────         ────────────────
PB13 (SCK)  ──────►   Pin 6 (CLK)
PB5  (MOSI) ──────►   Pin 5 (DI)
PB4  (MISO) ◄──────   Pin 2 (DO)
PB14 (CS_n) ──────►   Pin 1 (~CS)  ← 100kΩ pull-up a +3V3_DIG también
PB15 (WP_n) ──────►   Pin 3 (~WP)  ← o simplemente tirar a +3V3 si no usas WP
                        Pin 7 (~HOLD) ──► +3V3_DIG via 10kΩ
                        Pin 8 (VCC)  ──► +3V3_DIG + 100nF a DGND
                        Pin 4 (GND)  ──► DGND
```

### 11.2 OLED 128×64 SSD1306 — Display

Si se usa módulo breakout AliExpress: ya viene con pull-ups y condensadores.
Para PCB final integrado directamente:

```
SSD1306 (I2C 0x3C):
  VCC ──► +3V3_DIG + 100nF
  GND ──► DGND
  SCL ──► PB8 (I2C1_SCL con pull-up 4.7kΩ compartido con PCM5242)
  SDA ──► PB9 (I2C1_SDA con pull-up 4.7kΩ compartido con PCM5242)
  RES ──► GPIO de STM32 (reset del display, 100nF a GND)
```

> El I2C1 comparte bus entre tres ICs — verificar que no hay conflicto de dirección:
> - PCM5242:  `0x4C` (ADDR0 a GND)
> - SSD1306:  `0x3C`
> - PCA9685:  `0x40` (A0–A5 todos a GND)
> No hay conflicto. Velocidad: 400kHz (Fast Mode).

---

## 11.3 PCA9685 — LED Driver 16 Canales PWM

**Datasheet:** [nxp.com/pca9685](https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf)

### Por qué PCA9685 en lugar de WS2812B

Los teclados mecánicos modernos (Ducky, Keychron, etc.) usan exactamente este enfoque:
un IC driver dedicado por I2C con PWM por canal. Ventajas:
- **0 pines GPIO extras** — usa el I2C ya existente
- **PWM de 12 bits** (4096 niveles) por LED — control de brillo preciso
- **Sin timing estricto** — el I2C no tiene las restricciones de 800kHz del WS2812
- **Sin DMA dedicado** — libera recursos del STM32
- **Consumo total 16 LEDs** a 10mA c/u = 160mA (vs 960mA del WS2812 a pleno)

### Conexiones

```
STM32 I2C1 (PB8/PB9)          PCA9685 (TSSOP28, dirección 0x40)
─────────────────              ────────────────────────────────
PB8 (SCL) ─────────────────► Pin 19 (SCL)
PB9 (SDA) ─────────────────► Pin 18 (SDA)  ← pull-ups 4.7kΩ ya existen en el bus
PD12 (OE_n) ───────────────► Pin 20 (~OE)  ← tirar a DGND via 10kΩ (siempre activo)
                                              o conectar a GPIO para apagado global

Pin 1  (A0) ──► DGND  ┐
Pin 2  (A1) ──► DGND  │  dirección = 0x40
Pin 3  (A2) ──► DGND  │
Pin 4  (A3) ──► DGND  │
Pin 5  (A4) ──► DGND  │
Pin 6  (A5) ──► DGND  ┘
Pin 17 (VDD)  ──► +3V3_DIG + 100nF a DGND
Pin 16 (GND)  ──► DGND
Pin 21 (EXTCLK) ──► DGND (usa oscilador interno 25MHz)

Salidas (LED0–LED15, pines 10–15 y 22–28 según datasheet):
  Cada canal: Pin LED_n ──[330Ω 0402]──► LED+ → LED- ──► DGND
              Corriente por LED: (3.3V - Vf_LED) / 330Ω ≈ 10mA (Vf rojo/verde/azul ~2V)
              Para Vf = 2.0V: (3.3 - 2.0) / 330 = 3.9mA — suficiente para buena visibilidad
              Para más brillo: cambiar a 150Ω → ~8.7mA
```

### LEDs recomendados

| Color | Uso en secuenciador | Footprint | LCSC ejemplo |
|---|---|---|---|
| Verde | Paso activo normal | 0603 | C72043 |
| Rojo | Paso con mute o error | 0603 | C84256 |
| Amarillo | Paso con Parameter Lock | 0603 | C131268 |
| Azul | Paso seleccionado | 0603 | C72041 |

> Para máxima flexibilidad: colocar LEDs **bicolor rojo/verde** en cada paso
> (2 LEDs por posición en footprint 0603 lado a lado).
> El PCA9685 tiene exactamente 16 canales — si quieres bicolor necesitas 32 canales → **2× PCA9685**.
> Segundo PCA9685 en dirección `0x41` (A0 a +3V3, resto a GND).

### Código mínimo de inicialización

```c
// PCA9685 init — modo normal, 200Hz PWM (buena respuesta visual)
// Registro MODE1 = 0x00: sin sleep, usa oscilador interno
PCA9685_WriteReg(0x00, 0x00);  // MODE1: normal
// Registro PRE_SCALE para 200Hz: value = round(25MHz / (4096 × 200Hz)) - 1 = 29
PCA9685_WriteReg(0x00, 0x10);  // MODE1: sleep (para cambiar PRE_SCALE)
PCA9685_WriteReg(0xFE, 29);    // PRE_SCALE = 29 → ~200Hz
PCA9685_WriteReg(0x00, 0x00);  // MODE1: wake up

// Encender LED del paso activo (canal 4) a pleno brillo:
// Registros LED_n: 4 bytes por canal (ON_L, ON_H, OFF_L, OFF_H)
PCA9685_SetPWM(4, 0, 4095);   // paso 4: pleno brillo
PCA9685_SetPWM(3, 0, 512);    // paso 3: dimmed (12% brillo)
PCA9685_SetPWM(5, 0, 0);      // paso 5: apagado
```

---

## 12. Conectividad MIDI DIN-5 y CV/Gate

### 12.1 MIDI IN (con optoacoplador 6N137)

```
DIN-5 MIDI IN:                   6N137
Pin 5 ──[220Ω]──[1N4148]──►   Anode (LED)
Pin 4 ──────────────────────►  Cathode (LED)

6N137:
  Pin 1 (+ VCC 3.3V)
  Pin 2 (Anode LED)   ← desde DIN-5 Pin 5 via 220Ω y diodo 1N4148
  Pin 3 (Cathode LED) ← desde DIN-5 Pin 4
  Pin 5 (GND)         ← DGND
  Pin 6 (Output Vo)   ──[10kΩ pull-up a +3V3]──► USART1_RX (PA10) del STM32
  Pin 7 (ENABLE)      ──► +3V3_DIG
  Pin 8 (VCC)         ──► +3V3_DIG + 100nF
```

### 12.2 MIDI OUT

```
STM32H7 USART1_TX (PA9)
  │
  ├──[220Ω]──► DIN-5 MIDI OUT Pin 5
  └──[220Ω]──► DIN-5 MIDI OUT Pin 4  ← los 220Ω son estándar MIDI DIN spec
               DIN-5 MIDI OUT Pin 2 ──► +5V_DIG via 220Ω
```

### 12.3 CV/Gate Salida (Jacks 3.5mm TRS)

Las 4 salidas del DAC8564 (buffereadas por OPA2134/NE5532) van directamente a jacks TRS:
- **Tip** = señal CV (0 a 5V = 0 a 5 octavas @ 1V/oct)
- **Sleeve** = GND analógico (AGND)
- Ring = NC (o puede usarse para gate en formato que lo admite)

Protección en cada jack de salida: diodo **BAT54S** (doble schottky SOT-23) de +5V a AGND —
evita que un rack Eurorack inyecte voltaje hacia atrás en los amplificadores de salida.

---

## 13. Reglas de Layout PCB

### 13.1 Organización de Zonas

```
┌────────────────────────────────────────────────┐  100×100mm (límite JLCPCB tier 1)
│  [ZONA DIGITAL]              [ZONA ANALÓGICA]  │
│                                                 │
│  STM32H743  USB   Flash      PCM5242  AS3320    │
│  Cristal    OLED  MCP23017   THAT2180 OPA2134   │
│  SWD conn.  LEDs             DAC8564            │
│                                                 │
│  [ZONA POTENCIA]                                │
│  USB-C  ADP3335  LT3042  LT3094  LT1054        │
│  (abajo esquina)  (margen del plano analógico)  │
│                                                 │
│  ╔══════════════╗ ┊ ╔════════════════════════╗  │
│  ║ PLANO DGND   ║ ┊ ║ PLANO AGND             ║  │
│  ╚══════════════╝ ┊ ╚════════════════════════╝  │
│                   ┊                             │
│              STAR GROUND aquí: jumper 0Ω        │
└────────────────────────────────────────────────┘
```

### 13.2 Reglas Absolutas

1. **Planos de tierra:** DGND y AGND como planos de cobre relleno en capa Bottom.
   Se unen en UN SOLO PUNTO físico (jumper 0Ω o via directa en el esquemático).

2. **Cristal:** < 5mm del STM32. Plano DGND completo bajo el cristal.
   Nada de pistas de señal pasando por esa área.

3. **Decoupling capacitors:** Colocar a menos de 0.5mm del pin de VDD del IC.
   La via al plano de GND debe salir del pad del condensador, no del pad del IC.

4. **Zona LT1054 (charge pump):** Keep-out de señales analógicas en radio de 15mm.
   No trazar pistas de audio cerca del nodo de oscilación interna del LT1054.

5. **Líneas USB (D+ y D-):** Rutar en par diferencial, 90Ω impedancia diferencial,
   largo máximo 50mm, igual longitud entre D+ y D-.
   Mantener plano DGND bajo todo el recorrido, sin cortes.

6. **DAC8564 salidas:** Trazar por el lado analógico, alejadas del cristal y USB.
   Cada salida con condensador 100nF a AGND lo más cerca posible del pin VOUT.

7. **AS3320 socket DIP16:** Usar socket de calidad (Mill-Max o Enplas).
   El AS3320 puede necesitar reemplazarse — socket obligatorio.

8. **Trimpots:** Orientarlos hacia el borde de la PCB o hacia un panel de acceso.
   Documentar en el silkscreen qué ajusta cada trimpot.

9. **Via stitching:** Añadir vias de GND (M3 taladros) en el perímetro de la PCB cada 5mm.

10. **Net classes en KiCad:**
    - `AUDIO`: 0.2mm mínimo, clearance 0.3mm
    - `CV`: 0.2mm mínimo
    - `POWER_12V`: 1.0mm mínimo
    - `POWER_5V`: 0.5mm mínimo
    - `DIFFERENTIAL_USB`: 0.15mm + 0.15mm spacing para 90Ω

---

## 14. Checklist antes de exportar Gerbers

### Esquemático (antes del layout)
- [ ] ERC sin errores (solo warnings de PWR_FLAG son aceptables si se pusieron PWR_FLAG)
- [ ] Todos los VDD de cada IC tienen condensador de decoupling en el esquemático
- [ ] VCAP1 y VCAP2 del STM32H7 tienen 2.2µF y van a DGND (NO a VDD)
- [ ] LDAC_n del DAC8564 está conectado a un GPIO del STM32 (NO a GND fijo)
- [ ] Cristal tiene resistor serie 22Ω en OSC_IN
- [ ] Pull-down 10kΩ en BOOT0
- [ ] Cada encoder tiene pull-ups 10kΩ y condensadores anti-rebote 100nF
- [ ] Polyfuse 500mA en VBUS del USB
- [ ] USBLC6-2 en D+ y D-
- [ ] El AS3320 tiene resistor tempco en serie con el conversor V/I de ICTL
- [ ] Los potenciómetros van referenciados a +3V3 (NO a +5V)

### Layout (antes de Gerber)
- [ ] DRC sin errores
- [ ] AGND y DGND son planos separados con único punto de unión
- [ ] Cristal a menos de 5mm del STM32H7
- [ ] Decoupling capacitors a menos de 0.5mm de su IC
- [ ] Ninguna pista de audio paralela o cerca del LT1054 (charge pump)
- [ ] Capas F.Paste activas en todos los footprints de ICs SMD (para stencil)
- [ ] Silkscreen sin texto encima de pads
- [ ] Thermal vias bajo LT3042 y LT3094 (pads térmicos a AGND)
- [ ] Socket DIP16 footprint para el AS3320 (NOT soldered direct)
- [ ] Trimpots accesibles desde arriba del PCB
- [ ] Conector SWD accesible desde el exterior del chasis

### Exportación para JLCPCB
- [ ] Gerbers: F.Cu, B.Cu, F.Silkscreen, B.Silkscreen, F.Courtyard, Edge.Cuts, F.Paste
- [ ] Drill files: PTH + NPTH en formato Excellon
- [ ] BOM con Reference, Value, Footprint, LCSC Part Number (si aplica)
- [ ] CPL (Component Placement List) si se pide ensamblaje
- [ ] Dimensiones confirmadas en archivo Edge.Cuts: máximo 100×100mm para tier $2

### Específico USB-C Power
- [ ] Resistores 5.1kΩ conectados a CC1 y CC2 del conector USB-C (NO a +5V, a GND)
- [ ] Conector USB-C power es diferente al USB-B de MIDI — son dos conectores distintos
- [ ] Polyfuse 2A en VBUS del USB-C de potencia
- [ ] Verificar que LT1054 (no MAX1044) esté en el BOM para el rail negativo
- [ ] LT3094 en el BOM — no confundir con LT3042 (diferente IC, diferente pinout)
- [ ] PCA9685: verificar dirección I2C `0x40` no colisiona con otros ICs del bus
- [ ] Resistores 330Ω (o 150Ω para más brillo) en cada LED — calcular Vf del LED elegido
- [ ] Pin ~OE del PCA9685 con pull-down 10kΩ a DGND (LEDs activos por defecto al encender)

---

## APÉNDICE A — Comandos STM32CubeMX

Crear proyecto nuevo:
1. `File → New Project → Search: STM32H743VIT6`
2. Pinout & Configuration → `RCC → HSE → Crystal/Ceramic Resonator`
3. Clock Configuration → ingresar valores PLL1 y PLL3 según sección 3.5
4. `USB_OTG_FS → Device_Only → Full Speed`
5. `SAI1 → Mode: Asynchronous Master Transmit → I2S PCM Format Long Frame`
6. `SPI1 → Full-Duplex Master → Hardware NSS Disabled (gestión manual CS via GPIO)`
7. `SPI2 → Full-Duplex Master` (para Flash W25Q128)
8. `I2C1 → I2C → Fast Mode 400kHz`
9. `USART1 → Asynchronous, 31250 baud, 8-N-1` (MIDI DIN)
10. `Project Manager → Toolchain: STM32CubeIDE → Generate Code`

## APÉNDICE B — Orden Recomendado de Diseño en KiCad

1. Crear el proyecto KiCad
2. Agregar todos los símbolos custom que necesites (AS3320 no está en la lib estándar)
3. Dibujar bloque de alimentación completo + ERC
4. Dibujar bloque STM32H7 + cristal + SWD + decoupling completo + ERC
5. Dibujar bloque USB + protección + USBLC6-2 + ERC
6. Dibujar bloque PCM5242 + salida analógica + ERC
7. Dibujar bloque DAC8564 + buffers OPA2134 + ERC
8. Dibujar bloque AS3320 + conversores V/I + ERC
9. Dibujar bloque THAT2180 + ERC
10. Dibujar bloque multiplexores + pots + encoders + ERC
11. Dibujar bloque Flash + OLED + PCA9685 + LEDs + ERC
12. Dibujar bloque MIDI DIN + CV jacks + ERC
13. ERC final completo → 0 errores
14. Asignar footprints a todos los símbolos
15. Layout PCB en orden: zonas de tierra → potencia → digital → analógico → conectores
16. DRC → 0 errores
17. Revisar visualmente con 3D viewer
18. Exportar Gerbers
