# Guía de Diseño Esquemático
## DigitalSynth v1.0 — Referencia Completa para KiCad

> **v2.0 — Arquitectura 100% Digital (pivote desde HybridSynth):**
> Se eliminaron los bloques AS3320 (VCF) y THAT2180 (VCA) del diseño.
> El filtro y VCA se implementan como DSP en STM32H743.
> Las secciones 6 y 7 de esta guía quedan **obsoletas** — marcadas como `[ELIMINADO]`.
>
> **Alimentación simplificada:** Sin LT1054, sin LT3094. Solo +3.3V_DIG (ADP3335) y
> +3.3V_ANA (LT3042 a 3.3V para PCM5122). El USB-C entrega 5V @ 1.5A.
> Un resistor de 5.1kΩ en CC1/CC2 del conector USB-C hace que cualquier cargador
> moderno entregue automáticamente 5V @ 1.5A.
> El conector USB-B separado sigue siendo el puerto MIDI hacia el PC/DAW.

> **Metodología:** Ir módulo por módulo. Terminar y verificar ERC de cada bloque antes de
> continuar con el siguiente. Nunca avanzar con warnings sin resolver.

---

## ÍNDICE

1. [Resumen de ICs y módulos](#1-resumen-de-ics-y-módulos)
2. [Alimentación — Top Level](#2-alimentación--top-level)
3. [STM32H743 — MCU Principal](#3-stm32h743--mcu-principal)
4. [PCM5122 — DAC de Audio I2S](#4-pcm5122--dac-de-audio-i2s)
5. [DAC8565 — DAC CV SPI — CV Out externo](#5-dac8565--dac-cv-spi--cv-out-externo)
6. [AS3320 — VCF Analógico **\[ELIMINADO\]**](#6-as3320--vcf-analógico-eliminado)
7. [THAT2180 — VCA Analógico **\[ELIMINADO\]**](#7-that2180--vca-analógico-eliminado)
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
| U2 | PCM5122PW | DAC audio I2S 32-bit | TSSOP28 | [ti.com/pcm5122](https://www.ti.com/lit/ds/symlink/pcm5122.pdf) |
| U3 | DAC8565IAPWR | DAC CV 16-bit 4ch SPI, Vref interna 2.5V | TSSOP16 | [ti.com/dac8565](https://www.ti.com/lit/ds/symlink/dac8565.pdf) |
| ~~U4~~ | ~~AS3320~~ | ~~VCF Moog Ladder -24dB~~ | ~~DIP16~~ | **ELIMINADO** — filtro SVF en firmware |
| ~~U5~~ | ~~THAT2180LA14-U~~ | ~~VCA log dB/V~~ | ~~DIP8~~ | **ELIMINADO** — VCA en firmware |
| ~~U6~~ | ~~OPA2134UA~~ | ~~Op-amp audio dual~~ | ~~DIP8~~ | **ELIMINADO** |
| ~~U7~~ | ~~NE5532P~~ | ~~Op-amp audio dual~~ | ~~DIP8~~ | **ELIMINADO** |
| U8 | LT3042EMSE | LDO +3.3V_ANA para PCM5122 | MSOP10 | [analog.com/lt3042](https://www.analog.com/media/en/technical-documentation/data-sheets/3042fc.pdf) |
| ~~U9~~ | ~~LT3094EMSE~~ | ~~LDO -5V analógico~~ | ~~MSOP10~~ | **ELIMINADO** — sin rail negativo |
| U10 | ADP3335ARMZ-3.3 | LDO 3.3V digital | MSOP8 | [analog.com/adp3335](https://www.analog.com/media/en/technical-documentation/data-sheets/ADP3335.pdf) |
| ~~U11~~ | ~~LT1054ISW~~ | ~~Charge pump 5V→-5.5V~~ | ~~SOIC8~~ | **ELIMINADO** — sin rail negativo |
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
          ├──► [Ferrite 600Ω]──► [LT3042 LDO @ 3.3V] ──► +3V3_ANA (200mA)
          │                                               └──► PCM5122 AVDD + CPVDD
          │
          ├──► [Ferrite 600Ω] ──► +5V_BUS_FILT ─────────────────────────────────
          │                        └──► DAC8565 AVDD (CV out — ruido no crítico)
          │
          ├──► [ADP3335-3.3 LDO 5V→3.3V] ──► +3V3_DIG (500mA)
          │                                      └──► STM32H7, lógica, Flash, OLED
          │
          (SIN LT1054. SIN LT3094. SIN rail -5V_ANA.)
```

### Presupuesto de corriente (total desde +5V_USB)

| Subsistema | Corriente típica |
|---|---|
| STM32H743 @ 480MHz + periféricos | ~130mA |
| Lógica (MCP23017, PCA9685, W25Q128, OLED) | ~30mA |
| PCM5122 (AVDD @ 3.3V_ANA + DVDD @ 3.3V_DIG) | ~20mA |
| DAC8565 (CV out externo) | ~5mA |
| PCA9685 + 16× LED SMD 0603 (todos encendidos pleno) | ~170mA |
| **TOTAL TÍPICO** | **~355mA** |
| **PICO (todos LEDs pleno + carga máxima)** | **~400mA** |

> **Plan de corriente v2.0 (sin cadena analógica):**
> - Sin LT1054 + LT3094 se ahorran ~75mA de overhead de charge pump y ~60mA de ICs analógicos.
> - Total ahorro: ~135mA vs v1.0. El sistema ahora consume < 400mA pico.
> - Con **5.1kΩ en CC1+CC2**: el cargador USB-C entrega 5V @ **1.5A** — margen de 1.1A libre.
> - **NO usar puertos USB-A tipo 2.0** para alimentar — solo 500mA, insuficiente.

### Símbolo de net en KiCad para las líneas de poder:
Usar **PWR_FLAG** en cada red de alimentación para suprimir warnings de ERC.

| Net name | Voltaje | Corriente máx. | Fuente |
|---|---|---|---|
| `+5V_USB` | +5V | 1.5–3A | Conector USB-C |
| `+5V_BUS` | +5V | 1.5A | Post-polyfuse |
| `+3V3_DIG` | +3.3V | 500mA | ADP3335 LDO |
| `+3V3_ANA` | +3.3V | 200mA | LT3042 LDO (para PCM5122) |
| ~~`+5V_ANA`~~ | ~~+5V~~ | ~~200mA~~ | **ELIMINADO** — sin cadena analógica |
| ~~`-5V_ANA`~~ | ~~-5V~~ | ~~100mA~~ | **ELIMINADO** — sin LT1054/LT3094 |
| `AGND` | 0V analógico | — | Plano separado (solo PCM5122) |
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
| PE2 | SAI1_MCLK_A | SAI1 | I2S_MCLK | MCLK para PCM5122 |
| PE3 | SAI1_SD_B | SAI1 | I2S_SD | Data → PCM5122 DIN |
| PE4 | SAI1_FS_A | SAI1 | I2S_LRCK | Word clock → PCM5122 LRCK |
| PE5 | SAI1_SCK_A | SAI1 | I2S_BCK | Bit clock → PCM5122 BCK |
| PA5 | SPI1_SCK | SPI1 | DAC_SCK | DAC8565 SCLK |
| PA6 | *(no usado)* | — | — | DAC8565 no tiene SDO |
| PA7 | SPI1_MOSI | SPI1 | DAC_MOSI | DAC8565 DIN |
| PB6 | SPI1_NSS (via GPIO) | GPIO OUT | DAC_CSn | CS activo LOW |
| PB7 | GPIO OUT | GPIO OUT | DAC_LDAC | Flanco de SUBIDA actualiza los 4ch |
| PB4 | SPI2_MISO | SPI2 | FLASH_MISO | W25Q128 DO |
| PB5 | SPI2_MOSI | SPI2 | FLASH_MOSI | W25Q128 DI |
| PB13 | SPI2_SCK | SPI2 | FLASH_SCK | W25Q128 CLK |
| PB14 | GPIO OUT | GPIO OUT | FLASH_CSn | Activo LOW |
| PB8 | I2C1_SCL | I2C1 | I2C_SCL | OLED + PCM5122 + PCA9685 (bus compartido) |
| PB9 | I2C1_SDA | I2C1 | I2C_SDA | OLED + PCM5122 + PCA9685 (bus compartido) |
| PB3 | TIM2_CH2 (o GPIO) | ENCODER | ENC1_A | Encoder rotativo 1 |
| PA15 | TIM2_CH1 | ENCODER | ENC1_B | Encoder rotativo 1 |
| PC6..PC12 | GPIO IN | GPIO | ENC2..ENC8 | Encoders 2–8 (TIM3/TIM4) |
| PC0 | GPIO IN (EXTI) | GPIO | MCP23017_INT | Interrupción MCP23017 INT_A (activo LOW) |
| UART1_TX | PA9 | USART1 | MIDI_TX | MIDI DIN OUT (via 220Ω) |
| UART1_RX | PA10 | USART1 | MIDI_RX | MIDI DIN IN (via 6N137) |

> **Nota SAI:** Usar SAI1 en modo master transmitter. El PCM5122 actúa como slave.
> STM32H7 genera BCK, LRCK y MCLK. Asegurarse de habilitar MCLK en CubeMX
> (el PCM5122 lo requiere ya que no tiene oscilador propio — depende de MCLK externo).

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

## 4. PCM5122 — DAC de Audio I2S

### 4.1 Datasheet
[TI PCM5122 — ti.com/lit/ds/symlink/pcm5122.pdf](https://www.ti.com/lit/ds/symlink/pcm5122.pdf)

### 4.2 Descripción
- DAC estéreo 32-bit / 384kHz, SNR 112dB, THD+N < -93dB
- Reemplaza PCM5242PW — **mismo footprint TSSOP28, mismos registros I2C, sin PLL externo necesario**
- Interfaz digital: I2S esclavo (recibe BCK, LRCK, DATA del STM32)
- Interfaz de configuración: I2C (dirección 0x4C o 0x4D según ADDR0)
- Salida analógica: diferencial y single-ended, voltaje máximo ±2.1V rms
- Alimentación: DVDD = 3.3V, AVDD = **3.3V** (LT3042 @ 3.3V — no 5V)
- Tiene oscilador RC interno — pero usar MCLK externo del STM32 para sincronía perfecta

### 4.3 Conexiones Esquemáticas

```
STM32H743 SAI1          PCM5122 (TSSOP28)
──────────────          ─────────────────
PE5 (I2S_BCK)  ──────► Pin 3  (BCK)   − Bit clock
PE4 (I2S_LRCK) ──────► Pin 4  (LRCK)  − Word clock (fs = 48kHz)
PE3 (I2S_SD)   ──────► Pin 5  (DIN)   − Audio data
PE2 (I2S_MCLK) ──────► Pin 2  (SCK)   − System clock (MCLK = 512×fs = 24.576MHz)

PB8 (I2C_SCL)  ──[4.7kΩ pull-up a +3V3]── Pin 17 (SCK/I2C) − Config I2C
PB9 (I2C_SDA)  ──[4.7kΩ pull-up a +3V3]── Pin 16 (SDA/I2C) − Config I2C
PC3 (XSMT)     ──[10kΩ pull-up a +3V3]─── Pin 25 (XSMT)    − Soft mute (LOW = silencio)

Pin 22 (ADDR0)  ──── DGND              → Dirección I2C = 0x4C
Pin 18 (DVDD)   ──── +3V3_DIG + 100nF a DGND + 4.7µF
Pin 19 (DGND)   ──── DGND
Pin 1  (AVDD)   ──── +3V3_ANA (LT3042) + 100nF a AGND + 4.7µF
Pin 21 (AGND)   ──── AGND (IMPORTANTE: pin agnd va al plano analógico)
Pin 23 (CPVDD)  ──── +3V3_ANA + 10µF + 100nF (charge pump supply — MISMA red que AVDD)
Pin 24 (VCOM)   ──── 10µF a AGND   ← condensador de desacoplo del VCOM interno

— Salida de audio (señal diferencial → filtro pasivo → jack) —
Pin 27 (OUTP_L) ──[470Ω]──┬──[100Ω]──► TRS 6.35mm Tip (canal L)
Pin 28 (OUTN_L) ── AGND   └──[100Ω]──► RCA Izquierda (bifurcación)
Pin 25 (OUTP_R) ──[470Ω]──┬──[100Ω]──► TRS 6.35mm Ring (canal R)
Pin 26 (OUTN_R) ── AGND   └──[100Ω]──► RCA Derecha
                  Filtro anti-alias: 2.2nF a AGND en el nodo central (después de 470Ω)

```

> **Nota:** El PCM5122 produce salida single-ended desde OUTP (OUTN se termina a AGND).
> El filtro pasivo 470Ω + 2.2nF elimina componentes a alta frecuencia.
> La bifurcación con 100Ω aísla la carga de TRS del RCA — no se necesita op-amp de salida.

### 4.4 Inicialización I2C obligatoria

El PCM5122 arranca en modo de clock automático, pero hay que configurarle la frecuencia
exacta de oversampling via I2C. Con MCLK = 24.576MHz y fs = 48kHz: OSC = 512×fs.

```c
// Registro 37 (0x25): I2S data format → 32-bit I2S
PCM5122_WriteReg(0x25, 0x03);  // 32-bit, I2S format, MSB first
// Registro 19 (0x13): Clock source → MCLK (no requiere PLL interno)
PCM5122_WriteReg(0x13, 0x10);  // MCLK divider reference = MCLK input
// Registro 2 (0x02): Power state → run
PCM5122_WriteReg(0x02, 0x00);  // release standby, normal operation
// XSMT: soft-mute desactivado — poner PC3 HIGH antes de reproducir
```

---

## 5. DAC8565 — DAC CV SPI — CV Out externo

### 5.1 Datasheet
[TI DAC8565 — ti.com/lit/ds/symlink/dac8565.pdf](https://www.ti.com/lit/ds/symlink/dac8565.pdf)

### 5.2 Descripción
- DAC de 16 bits, 4 canales independientes, interfaz SPI (hasta 50MHz)
- **Referencia interna 2.5V** — salida 0V a 2.5V por canal (sin Vref externo)
- No necesita amplificador de salida — salida direct-to-jack, rail-to-rail interno
- SPI: 24 bits por transferencia (8 bits control + 16 bits dato), **solo escritura — sin SDO**
- LDAC: **flanco de subida** actualiza todos los canales simultáneamente (invertido vs DAC8564)
- AVDD: 2.7V a 5.5V — se alimenta desde +5V_BUS_FILT o desde +3V3_DIG
- IOVDD: alimentación de la interfaz lógica SPI — debe seguir el nivel del STM32 (+3V3_DIG)

### 5.3 Conexiones Esquemáticas

```
STM32H743 SPI1          DAC8565 (TSSOP16)
──────────────          ─────────────────
PA5 (SPI_SCK)  ──────► SCLK            − Hasta 50MHz
PA7 (SPI_MOSI) ──────► SDI (DIN)       − Datos MSB-first (NO HAY SDO/MISO)
PB6 (DAC_CSn)  ──────► ~SYNC (~CS)     − Active LOW, bajar antes de cada trama
PB7 (DAC_LDAC) ──────► LDAC            − Mantener LOW mientras se escriben los 4 canales
                                          SUBIR A HIGH para actualizar simultáneamente

AVDD     ──── +5V_BUS_FILT (via ferrite) + 100nF a AGND + 4.7µF
IOVDD    ──── +3V3_DIG + 100nF a DGND   ← nivel lógico SPI = 3.3V
AGND     ──── AGND
GND      ──── AGND

VREF (interno 2.5V):
  Pin VREF ──[100nF a AGND]  ← bypass del ref. interno — NO conectar Vref externo
  Rango de salida: 0V a 2.5V (suficiente para 2.5 octavas @ 1V/oct por canal)

Pines de control:
  ENABLE  ──── AGND    (active LOW → siempre habilitado)
  RSTSEL  ──── AGND    (outputs resetean a 0V al arrancar)

Salidas CV (directo a jacks TRS 3.5mm, sin buffer op-amp):
  VOUTA  ──[100Ω]──► Jack CV1 Tip  (Pitch voz 1)  Sleeve = AGND
  VOUTB  ──[100Ω]──► Jack CV2 Tip  (Pitch voz 2)  Sleeve = AGND
  VOUTC  ──[100Ω]──► Jack CV3 Tip  (Mod/Env 1)    Sleeve = AGND
  VOUTD  ──[100Ω]──► Jack CV4 Tip  (Mod/Env 2)    Sleeve = AGND
  (100Ω limita corriente de cortocircuito; BAT54S de +5V a AGND en cada jack)
```

> **Sin buffer op-amp:** El DAC8565 tiene salida rail-to-rail de impedancia baja.
> El resistor de 100Ω en serie es suficiente para proteger el pin de salida si el
> cable CV se cortocircuita o si un rack Eurorack inyecta voltaje hacia atrás.
> No se necesita OPA2134 ni NE5532 — ICs eliminados en v2.0.

### 5.4 Formato SPI — 24 bits

```
Trama de 24 bits (MSB first):
  Bits [23:20]: Comando
    0b0001 (0x1n) → Write input reg canal A, sin actualizar salida
    0b0011 (0x3n) → Write input reg canal B
    0b0101 (0x5n) → Write input reg canal C
    0b0111 (0x7n) → Write input reg canal D
    0b1111 (0xFn) → Write input reg + actualizar todos los canales inmediatamente
  Bits [19:4]: Dato DAC 16-bit (MSB first)
  Bits [3:0]:  No importan (0x0)

Secuencia recomendada — actualizar 4 canales con LDAC:
  1. PB7 (LDAC) → LOW   (mantener low durante toda la secuencia)
  2. PB6 (CS_n) → LOW
  3. Enviar 24 bits canal A (comando 0x18 + 16 bits dato)
  4. PB6 → HIGH (latch canal A en input register)
  5. Repetir pasos 2-4 para canales B, C, D
  6. PB7 (LDAC) → HIGH  ← flanco de SUBIDA → actualiza los 4 canales
                            simultáneamente (glitch-free)

Cálculo de valor DAC para 1V/oct (rango 0–2.5V):
  cv_volts = note_semitones / 12.0f           // 1 octava = 1.0V
  dac_code = (uint16_t)(cv_volts / 2.5f * 65535.0f);  // rango 0x0000–0xFFFF
```

---

## 6. AS3320 — VCF Analógico `[ELIMINADO v2.0]`

> ⛔ **Esta sección está obsoleta.** El AS3320 fue eliminado del diseño en la versión 2.0.
> El filtro ahora se implementa como SVF (State Variable Filter) digital en el firmware del STM32H743.
> Ver `firmware/dsp/` para la implementación. Esta sección se mantiene como referencia histórica.

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

## 7. THAT2180 — VCA Analógico `[ELIMINADO v2.0]`

> ⛔ **Esta sección está obsoleta.** El THAT2180 fue eliminado del diseño en la versión 2.0.
> El VCA ahora se implementa como multiplicación digital de muestra por envolvente en el firmware.
> Esta sección se mantiene como referencia histórica.

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

### 9.2 LT3042 — LDO Ultra Bajo Ruido (+3.3V Analógico para PCM5122)

**Datasheet:** [analog.com/lt3042](https://www.analog.com/media/en/technical-documentation/data-sheets/3042fc.pdf)

```
+5V_BUS ──[Ferrite 600Ω@100MHz, 1A]──► LT3042 VIN
                                        LT3042 VOUT ──► +3V3_ANA
                                        LT3042 SET  ──[33kΩ a AGND]  ← fija Vout a 3.3V (ISET×RSET = 100µA×33k = 3.3V)
                                        LT3042 ILIM ──[360kΩ a AGND]  ← limita a ~200mA
                                        LT3042 EN   ──► +5V_BUS (siempre ON)
                                        LT3042 GND  ──► AGND
                                        C_IN:  10µF X5R 0805 + 100nF C0G a AGND
                                        C_OUT: 10µF X5R 0805 + 100nF C0G a AGND
                                        C_SET: 10nF a AGND (en el pin SET) ← reduce ruido

+3V3_ANA ──► PCM5122 AVDD (pin 1) + CPVDD (pin 23)
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

### 9.4 LT1054 — Charge Pump `[ELIMINADO v2.0]`

> ⛔ **Esta sección está obsoleta.** El LT1054 fue eliminado en v2.0 al no necesitar rail -5V.
> Se mantiene como referencia si en el futuro se añade una cadena analógica.

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

### 9.5 LT3094 — LDO Ultra Bajo Ruido (-5V Analógico) `[ELIMINADO v2.0]`

> ⛔ **Esta sección está obsoleta.** El LT3094 fue eliminado en v2.0 al no necesitar rail -5V.

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

### Resumen: ICs eliminados en v2.0 (digital-only)

| Eliminado | Motivo |
|---|---|
| AS3320 (VCF analógico) | Reemplazado por SVF digital en firmware |
| THAT2180 (VCA analógico) | Reemplazado por multiplicación digital de muestra |
| OPA2134 (buffer de salida y diferencial) | No necesario — PCM5122 salida directa a filter pasivo |
| NE5532 (buffer CV) | No necesario — DAC8565 rail-to-rail directo a jacks |
| LT3094 (LDO -5V) | Rail negativo eliminado — no hay circuitos analógicos bipolares |
| LT1054 (charge pump) | Rail negativo eliminado |
| Relay OMRON G6K-2F | No hay señales analógicas que conmutar |

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

CRÍTICO: NO usar +5V en el extremo CW — el ADC del STM32H743 es 3.3V máximo.
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
  SCL ──► PB8 (I2C1_SCL con pull-up 4.7kΩ compartido con PCM5122)
  SDA ──► PB9 (I2C1_SDA con pull-up 4.7kΩ compartido con PCM5122)
  RES ──► GPIO de STM32 (reset del display, 100nF a GND)
```

> El I2C1 comparte bus entre tres ICs — verificar que no hay conflicto de dirección:
> - PCM5122:  `0x4C` (ADDR0 a GND)
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

Las 4 salidas del DAC8565 van directamente a jacks TRS 3.5mm sin buffer op-amp:
- **Tip** = señal CV (0 a 2.5V — referencia interna DAC8565; compatible con Eurorack a 1V/oct = 2.5 oct)
- **Sleeve** = AGND
- Ring = NC

Protección en cada jack de salida: diodo **BAT54S** (doble schottky SOT-23) entre +5V_BUS_FILT y AGND —
evita que un rack Eurorack inyecte voltaje hacia atrás. El resistor 100Ω en serie en la salida
del DAC8565 limita la corriente en caso de cortocircuito.

---

## 13. Reglas de Layout PCB

### 13.1 Organización de Zonas

```
┌────────────────────────────────────────────────┐  100×100mm (límite JLCPCB tier 1)
│  [ZONA DIGITAL]                                    │
│                                                    │
│  STM32H743  USB    Flash    OLED    MCP23017       │
│  Cristal    SWD   W25Q128  PCA9685  Encoders       │
│                                                    │
│  [ZONA AUDIO/CV]                                   │
│  PCM5122  DAC8565  Ferrites  BAT54S                │
│  Filtro pasivo 470Ω+2.2nF  Jacks TRS/RCA/CV      │
│                                                    │
│  [ZONA POTENCIA]                                   │
│  USB-C  ADP3335  LT3042  (sin LT3094, sin LT1054) │
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

4. **Líneas USB (D+ y D-):** Rutar en par diferencial, 90Ω impedancia diferencial,
   largo máximo 50mm, igual longitud entre D+ y D-.
   Mantener plano DGND bajo todo el recorrido, sin cortes.

5. **DAC8565 salidas CV:** Trazar por la zona audio/CV, alejadas del cristal y USB.
   Cada salida con condensador 100nF a AGND lo más cerca posible del pin VOUT.
   Resistor 100Ω en serie a menos de 5mm del pin de salida.

6. **PCM5122 zona audio:** AVDD y CPVDD deben tener sus condensadores lo más cerca del IC.
   El pin AGND (pin 21) debe conectarse directamente al plano AGND, no via DGND.

7. **Via stitching:** Añadir vias de GND en el perímetro de la PCB cada 5mm.

8. **Net classes en KiCad:**
    - `AUDIO`: 0.2mm mínimo, clearance 0.3mm
    - `CV`: 0.2mm mínimo
    - `POWER_5V`: 0.5mm mínimo
    - `POWER_3V3`: 0.5mm mínimo
    - `DIFFERENTIAL_USB`: 0.15mm + 0.15mm spacing para 90Ω

---

## 14. Checklist antes de exportar Gerbers

### Esquemático (antes del layout)
- [ ] ERC sin errores (solo warnings de PWR_FLAG son aceptables si se pusieron PWR_FLAG)
- [ ] Todos los VDD de cada IC tienen condensador de decoupling en el esquemático
- [ ] VCAP1 y VCAP2 del STM32H7 tienen 2.2µF y van a DGND (NO a VDD)
- [ ] LDAC del DAC8565 está conectado a un GPIO del STM32 (PB7) — flanco de SUBIDA para actualizar
- [ ] XSMT del PCM5122 (pin 25) conectado a PC3 con pull-up 10kΩ a +3V3
- [ ] Cristal tiene resistor serie 22Ω en OSC_IN
- [ ] Pull-down 10kΩ en BOOT0
- [ ] Cada encoder tiene pull-ups 10kΩ y condensadores anti-rebote 100nF
- [ ] Polyfuse 500mA en VBUS del USB
- [ ] USBLC6-2 en D+ y D-
- [ ] Los potenciómetros van referenciados a +3V3 (NO a +5V)

### Layout (antes de Gerber)
- [ ] DRC sin errores
- [ ] AGND y DGND son planos separados con único punto de unión
- [ ] Cristal a menos de 5mm del STM32H7
- [ ] Decoupling capacitors a menos de 0.5mm de su IC
- [ ] Capas F.Paste activas en todos los footprints de ICs SMD (para stencil)
- [ ] Silkscreen sin texto encima de pads
- [ ] Thermal vias bajo LT3042 (pad térmico a AGND)
- [ ] PCM5122 AGND (pin 21) conectado directamente al plano AGND
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
- [ ] LT3042 configurado para 3.3V (RSET = 33kΩ) — NO confundir con la versión de 5V
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
2. Agregar símbolos custom si es necesario (PCM5122 y DAC8565 están en la lib TI de KiCad)
3. Dibujar bloque de alimentación completo + ERC
4. Dibujar bloque STM32H7 + cristal + SWD + decoupling completo + ERC
5. Dibujar bloque USB + protección + USBLC6-2 + ERC
6. Dibujar bloque PCM5122 + filtro de salida pasivo + jacks TRS/RCA + ERC
7. Dibujar bloque DAC8565 + resistores CV 100Ω + jacks TRS 3.5mm + ERC
8. Dibujar bloque encoders + pots + MCP23017 + ERC
9. Dibujar bloque Flash + OLED + PCA9685 + LEDs + ERC
10. Dibujar bloque MIDI DIN + CV jacks + ERC
11. ERC final completo → 0 errores
12. Asignar footprints a todos los símbolos
13. Layout PCB en orden: zonas de tierra → potencia → digital → audio/CV → conectores
14. DRC → 0 errores
15. Revisar visualmente con 3D viewer
16. Exportar Gerbers
16. DRC → 0 errores
17. Revisar visualmente con 3D viewer
18. Exportar Gerbers

---

## APÉNDICE C — Videos y Recursos de Aprendizaje

> Ordenados por área técnica. Ver el orden recomendado en C.9.

---

### C.1 KiCad — Esquemático y Layout PCB

**Canal principal: [Phil's Lab](https://www.youtube.com/@PhilsLab)**
IPC CID+ certified, especialista en DSP + audio PCB. Es el recurso más directamente aplicable para este proyecto.

| Video | URL | Por qué verlo |
|---|---|---|
| KiCad 8 STM32 PCB Design – Full Tutorial | https://www.youtube.com/watch?v=aVUqaB0IMh4 | STM32 + KiCad end-to-end, exactamente el stack de este proyecto |
| Designing a DSP Audio Board in KiCad | https://www.youtube.com/watch?v=14_jh3nLSsU | PCB de audio con DAC I2S, signal integrity para audio |
| USB-C Power Delivery – Hardware Design | https://www.youtube.com/watch?v=eFKeNPJq2zs | CC1/CC2, polyfuse, VBUS — sección de power de este proyecto |
| STM32 Schematic Design (clock, decoupling, SWD) | https://www.youtube.com/watch?v=pm0WnQm_BVHQ | VCAP, cristal 25MHz, reset, SWD — bloque MCU completo |
| PCB Layout Checklist – Mixed-Signal | https://www.youtube.com/watch?v=VXE_dh7XYDI | AGND/DGND, star ground, rutas analógicas vs digitales |
| Power Supply Design for Audio (LDO Selection) | https://www.youtube.com/watch?v=Zrn5H1_t1kQ | LDO para audio, PSRR — LT3042/LT3094 de este proyecto |
| Real-Time DSP on STM32 | https://www.youtube.com/watch?v=6d1GvFVK3H8 | DMA double buffer + CMSIS-DSP — arquitectura SAI1 de este proyecto |

---

### C.2 Teclado Mecánico — PCB (Cherry MX + Encoders)

Directamente aplicable para los 16x Cherry MX y 8x encoders EC11 en KiCad.

| Recurso | URL | Por qué verlo |
|---|---|---|
| Joe Scotto – Custom Keyboard PCB from Scratch | https://www.youtube.com/watch?v=8WXpGTIbxlQ | Cherry MX footprints, matrix switches, encoders en KiCad |
| ai03 – How to Design a Custom Keyboard PCB | https://www.youtube.com/watch?v=BhFqkVggv8Q | Proceso completo con MCU, aplica igual para STM32 |
| ai03 PCB Designer Guide (guía escrita, 7 partes) | https://wiki.ai03.com/books/pcb-design | KiCad keyboard PCB con USB-C y LEDs |

---

### C.3 Sintetizador Analógico DIY — VCF / VCA / Circuitos

**Canal: [Moritz Klein](https://www.youtube.com/@MoritzKlein0)**
El mejor recurso pedagógico para entender el AS3320 y THAT2180.

| Video | URL | Por qué verlo |
|---|---|---|
| VCO from Scratch (playlist completa) | https://www.youtube.com/playlist?list=PLHeL0JWdJLvTuGCyC3qvx0RL8W5V9R-PT | Pedagogía: cada componente explicado antes de usarlo |
| MS-20 Filter Clone | https://www.youtube.com/watch?v=v3rMVTOFCVE | VCF analógico OTA — topología del filtro activo |
| DIY Moog Ladder Filter | https://www.youtube.com/watch?v=VL6XVKPBP4I | Misma topología que el AS3320, explicado visualmente |

**Georgia Tech — Electronics for Music Synthesis (Aaron Lanterman)**
Curso universitario completo sobre VCF/VCA/envelopes.

| Playlist | URL |
|---|---|
| EMS 2010 (48 videos) | https://www.youtube.com/playlist?list=PLXnFQqfHz_iHvvUlt5jV162WUmvnzpoLa |
| EMS 2011 | https://www.youtube.com/playlist?list=PLXnFQqfHz_iHHFepUHj2r-tWKyfouxX3H |

---

### C.4 STM32 Firmware y HAL — CubeMX / CubeIDE

**Canal: [Controllers Tech](https://www.youtube.com/@ControllersTech)**

| Video | URL | Por qué verlo |
|---|---|---|
| STM32 SAI – I2S Audio Output con DMA | https://www.youtube.com/watch?v=1WU5qIVRvN4 | SAI1 → PCM5122, DMA double buffer |
| STM32 SPI con DMA | https://www.youtube.com/watch?v=t6yhF4WcExM | SPI1 → DAC8565, SPI2 → W25Q128 |
| STM32 I2C – Master Transmit/Receive | https://www.youtube.com/watch?v=8m0RzBEhkHA | I2C bus: PCM5122 + SSD1306 + PCA9685 + MCP23017 |
| STM32 USB MIDI / CDC | https://www.youtube.com/watch?v=BmKe1hNs3tg | USB OTG FS class-compliant MIDI |
| STM32 Encoder Interface (TIM quadrature) | https://www.youtube.com/watch?v=fnrqRcOUe2c | 8x EC11 en modo quadrature con TIMx |

---

### C.5 Fuentes de Alimentación

| Video | URL | Por qué verlo |
|---|---|---|
| LDO Regulators – How They Work (Sam Ben-Yaakov) | https://www.youtube.com/watch?v=8LH0PVFf6TE | LT3042/LT3094: PSRR, caps, ruido de salida |
| Charge Pump Circuits (Sam Ben-Yaakov) | https://www.youtube.com/watch?v=I0rBJXBX3LQ | LT1054 explicado desde primeros principios |

---

### C.6 USB MIDI — Descriptores y Protocolo

| Video | URL | Por qué verlo |
|---|---|---|
| USB MIDI 1.0 Descriptor Deep Dive | https://www.youtube.com/watch?v=Ewag-DPNBUE | Formato del usb_midi_desc.c ya escrito en el proyecto |
| Class-Compliant USB Audio on STM32 | https://www.youtube.com/watch?v=3GklDzgE6GM | Integración de descriptores con HAL USB callbacks |

---

### C.7 Soldadura SMD

| Video | URL | Por qué verlo |
|---|---|---|
| SMD Soldering – Collin's Lab (Adafruit) | https://www.youtube.com/watch?v=fYInlAmPnGo | TSSOP28, MSOP, LQFP100 — todos los ICs de este proyecto |
| EEVblog #997 – SMD Soldering Tutorial Part 3 | https://www.youtube.com/watch?v=b9FC9fAlfQE | Técnica práctica de soldadura SMD casera |

---

### C.8 Recursos Escritos (No-Video)

| Recurso | URL | Para qué |
|---|---|---|
| r/PrintedCircuitBoard Wiki | https://www.reddit.com/r/PrintedCircuitBoard/wiki/index/ | Review checklist antes de enviar Gerbers |
| Synth DIY Wiki (sdiy.info) | https://sdiy.info/wiki/Main_Page | Base de conocimiento completa de synth DIY |
| Modwiggler – Music Tech DIY | https://www.modwiggler.com/forum/viewforum.php?f=17 | Comunidad profesional synth DIY, peer review de esquemáticos |
| musicdsp.org | https://musicdsp.org/en/latest/ | Algoritmos de síntesis FM, ADSR, filtros digitales |
| r/synthdiy Wiki | https://www.reddit.com/r/synthdiy/wiki/resources/ | Libros, proyectos y comunidades DIY curados |

---

### C.9 Orden Recomendado de Visionado

```
ANTES DE ABRIR KICAD:
  1.  Phil's Lab → KiCad 8 STM32 PCB Design Full Tutorial
  2.  Phil's Lab → STM32 Schematic Design (clock, decoupling, SWD)

MIENTRAS DISEÑAS EL ESQUEMATICO:
  3.  Phil's Lab → USB-C Power Design
  4.  Phil's Lab → Mixed-Signal PCB Layout Checklist
  5.  Joe Scotto → Custom Keyboard PCB (Cherry MX + encoders en KiCad)
  6.  ai03 PCB Designer Guide (leer en paralelo con KiCad abierto)

MIENTRAS ESPERAS EL PCB (firmware):
  7.  Controllers Tech → STM32 SAI I2S + DMA
  8.  Controllers Tech → STM32 I2C + SPI
  9.  Controllers Tech → STM32 Encoder (TIM quadrature)
  10. Phil's Lab → Real-Time DSP on STM32

CUANDO LLEGUEN LAS PLACAS (ensamblaje):
  11. EEVblog → SMD Soldering Tutorial
  12. Collin's Lab → SMD Soldering

OPCIONAL — CONTEXTO HISTÓRICO DE SÍNTESIS ANALÓGICA:
  13. Moritz Klein → DIY Moog Ladder Filter (para entender qué hace el SVF digital)
  14. Moritz Klein → VCO/VCF series completa
  15. Aaron Lanterman → Georgia Tech EMS (profundidad teórica de síntesis)
```
