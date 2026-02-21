# Pinout Completo — STM32H743VIT6
## HybridSynth v1.0 — Referencia para STM32CubeMX

---

## ÍNDICE

1. [Resumen General](#resumen-general)
2. [Pines de Sistema — No GPIO](#pines-de-sistema--no-gpio)
3. [SWD — Debug y Programación](#swd--debug-y-programación)
4. [USB OTG FS — MIDI USB](#usb-otg-fs--midi-usb)
5. [USART1 — MIDI DIN](#usart1--midi-din)
6. [SAI1 — Audio I2S → PCM5242](#sai1--audio-i2s--pcm5242)
7. [SPI1 — DAC de CV → DAC8564](#spi1--dac-de-cv--dac8564)
8. [SPI2 — Flash de Presets → W25Q128](#spi2--flash-de-presets--w25q128)
9. [I2C1 — Bus Compartido](#i2c1--bus-compartido)
10. [ADC — Potenciómetros y CV Input](#adc--potenciómetros-y-cv-input)
11. [Encoders Rotativos EC11 — Hardware TIM](#encoders-rotativos-ec11--hardware-tim)
12. [Encoders Rotativos EC11 — Software GPIO](#encoders-rotativos-ec11--software-gpio)
13. [Push de Encoders — Botones](#push-de-encoders--botones)
14. [Resumen de Pines Usados / Libres](#resumen-de-pines-usados--libres)
15. [Checklist CubeMX — Paso a Paso](#checklist-cubemx--paso-a-paso)
16. [Conflictos y Resoluciones](#conflictos-y-resoluciones)

---

## 1. RESUMEN GENERAL

| Bloque funcional | Pines GPIO usados | Tipo de periférico |
|---|---|---|
| Sistema (cristal, VCAP, BOOT) | 0 GPIO (pines dedicados) | RCC, Power |
| SWD Debug | 2 | SWD |
| USB OTG FS (MIDI USB) | 2 | OTG_FS |
| USART1 (MIDI DIN) | 2 | USART |
| SAI1 (audio I2S) | 4 | SAI |
| SPI1 (DAC8564 — CV) | 5 (3 AF + 2 GPIO) | SPI + GPIO |
| SPI2 (W25Q128 — Flash) | 5 (4 AF + 1 GPIO) | SPI + GPIO |
| I2C1 (bus compartido) | 2 | I2C |
| I2C — GPIOs de control | 2 | GPIO |
| ADC (2 pots + 1 CV input) | 3 | ADC |
| PCM5242 PDN (power down) | 1 | GPIO |
| Encoders hardware (TIM quadrature) | 8 (4 timers × 2 pines) | TIM |
| Encoders software (GPIO + EXTI) | 8 (4 encoders × 2 pines) | GPIO |
| Push de encoders (8 botones) | 8 | GPIO |
| **TOTAL pines de señal usados** | **52** | — |
| Pines libres en LQFP100 (~82 GPIO) | **~30** | Expansión futura |

> El STM32H743VIT6 en LQFP100 tiene aproximadamente 82 pines de I/O disponibles.
> Con 52 usados quedan ~30 libres para futuras expansiones (teclado, pantalla, etc.)

---

## 2. PINES DE SISTEMA — NO GPIO

Estos pines **no se configuran como GPIO** en CubeMX — se asignan automáticamente
al habilitar los periféricos correspondientes. Son **obligatorios** en el esquemático.

| Pin STM32 | Nombre | Función | Valor/Componente | Notas |
|---|---|---|---|---|
| PH0 | OSC_IN | Cristal HSE | ABM3B-25.000MHz + 22 Ω serie | Entrada del oscilador 25 MHz |
| PH1 | OSC_OUT | Cristal HSE | ABM3B-25.000MHz | Salida — 12 pF a DGND en cada pin |
| VCAP1 | VCAP1 | LDO interno CPU | 4.7 µF cerámico a DGND | **Obligatorio** — sin él el MCU no arranca |
| VCAP2 | VCAP2 | LDO interno CPU | 4.7 µF cerámico a DGND | **Obligatorio** — RM0433 sección 5.3.2 |
| BOOT0 | BOOT0 | Modo de arranque | 10 kΩ a GND | LOW = arrancar desde Flash interna |
| nRST | NRST | Reset del sistema | 100 nF a GND + botón a GND | Pull-up interno siempre activo |
| VDDA | VDDA | Alimentación ADC | 1 µF + 10 nF C0G a AGND | Separado del VDD digital |
| VREF+ | VREF+ | Referencia ADC | 1 µF + 10 nF C0G a AGND | Conectar a VDDA si no hay ref externa |

**En CubeMX:**
- `RCC → HSE → Crystal/Ceramic Resonator`
- Los pines VCAP, BOOT0 y nRST son pines físicos en el footprint — poner condensadores
  directamente debajo del IC en el layout.

---

## 3. SWD — DEBUG Y PROGRAMACIÓN

Permite programar y depurar el MCU con ST-Link V3 o J-Link.

| Pin STM32 | Función CubeMX | Net KiCad | Descripción |
|---|---|---|---|
| PA13 | SYS_SWDIO | SWDIO | Datos SWD bidireccionales |
| PA14 | SYS_SWCLK | SWDCK | Clock del debugger |

> **Nota:** PA13/PA14 quedan reservados **permanentemente** para SWD. No asignar a
> ninguna otra función aunque CubeMX lo permita — perderías la capacidad de flashear.

> **SWO (PB3)** — La traza ITM/SWO (printf por SWD) usa PB3. En este proyecto
> PB3 está asignado al encoder 1 (TIM2_CH2). Elige uno u otro. Para producción
> desactiva SWO y usa PB3 para el encoder.

**Conector SWD recomendado en PCB:**
```
10 pines, 1.27mm pitch (Samtec FTSH-105-01-L-DV-K)
Pin 1 → +3V3_DIG
Pin 2 → SWDIO (PA13)
Pin 3 → GND
Pin 4 → SWDCK (PA14)
Pin 5 → GND
Pin 6 → SWO/PB3 (no conectar si se usa para encoder)
Pin 7 → KEY (sin conectar)
Pin 8 → NC
Pin 9 → GND
Pin 10 → nRESET
```

---

## 4. USB OTG FS — MIDI USB

El STM32H743 tiene un USB OTG FS integrado que aparece como dispositivo MIDI
Class-Compliant en Windows/Mac/Linux sin drivers.

| Pin STM32 | Función CubeMX | Net KiCad | Descripción |
|---|---|---|---|
| PA11 | USB_OTG_FS_DM | USB_DM | D- del bus USB — via USBLC6-2 (protección ESD) |
| PA12 | USB_OTG_FS_DP | USB_DP | D+ del bus USB — via USBLC6-2 (protección ESD) |

> **Sin VBUS detect:** PA9 (posible VBUS sense) está ocupado por USART1_TX.
> Solución: en CubeMX desactivar VBUS sensing → `USB_OTG_FS → Activate_VBUS = No`.
> El MCU asume siempre que hay host y activa el pull-up D+ por software.

**En CubeMX:**
- `USB_OTG_FS → Mode: Device_Only`
- `USB_OTG_FS → Activate_VBUS → No`
- En el clock: PLL3 debe generar exactamente 48 MHz para USB (ver doc `05_GUIA_ESQUEMATICO.md` sección 3.5)

---

## 5. USART1 — MIDI DIN

MIDI DIN clásico (5 pines, 31250 baud, 8-N-1).
TX = MIDI OUT físico; RX = MIDI IN físico (entrada desde otro instrumento).

| Pin STM32 | Función CubeMX | Net KiCad | Descripción |
|---|---|---|---|
| PA9 | USART1_TX | MIDI_TX | MIDI DIN OUT → resistencia 220 Ω → pin 5 del conector DIN-5 |
| PA10 | USART1_RX | MIDI_RX | MIDI DIN IN ← pin 4 del DIN-5 ← optoacoplador 6N137 |

**En CubeMX:**
- `USART1 → Mode: Asynchronous`
- Baud Rate: `31250`
- Word Length: `8 bits`
- Parity: `None`
- Stop Bits: `1`

---

## 6. SAI1 — AUDIO I2S → PCM5242

El STM32 actúa como **master** — genera todos los clocks. El PCM5242 es esclavo.
Se usa la interfaz SAI (Serial Audio Interface) que en el STM32H7 reemplaza al I2S clásico.

| Pin STM32 | Función CubeMX | Net KiCad | Va a PCM5242 pin | Descripción |
|---|---|---|---|---|
| PE2 | SAI1_MCLK_A | I2S_MCLK | Pin 2 (SCK) | Master Clock — 256× o 512× la frecuencia de muestreo |
| PE5 | SAI1_SCK_A | I2S_BCK | Pin 3 (BCK) | Bit Clock — 64× fs = 3.072 MHz a 48 kHz |
| PE4 | SAI1_FS_A | I2S_LRCK | Pin 4 (LRCK) | Frame Sync / Word Clock — 1× fs = 48 kHz |
| PE3 | SAI1_SD_B | I2S_SD | Pin 5 (DIN) | Datos de audio serie (MSB first, 32 bits/canal) |

> **¿Por qué SD_B y no SD_A?** El bloque A del SAI1 genera los clocks (MCLK, SCK, FS).
> El bloque B usa esos clocks para transmitir datos. En modo I2S estéreo solo se necesita
> un canal de datos — SD_B es la elección óptima según la nota de aplicación AN5086 de ST.

**En CubeMX:**
- `SAI1 → Block A: Master with no MCLK` → cambia a `Master TX` con MCLK habilitado
- `SAI1 → Block B: Synchronous with Block A → Transmitter`
- Protocol: `I2S` — Frame Format: `I2S Philips`
- Data Size: `32 bits`
- FIFO Threshold: `Half Full`
- Habilitar DMA para SAI1 (TX con DMA doble buffer en AXI SRAM)

---

## 7. SPI1 — DAC DE CV → DAC8564

El DAC8564 convierte valores digitales del MCU a voltajes analógicos (0–5V, 4 canales)
que controlan el VCF (AS3320) y VCA (THAT2180) a través de buffers OPA2134.

| Pin STM32 | Función CubeMX | Net KiCad | Va a DAC8564 pin | Descripción |
|---|---|---|---|---|
| PA5 | SPI1_SCK | DAC_SCK | Pin 8 (SCLK) | Clock SPI — hasta 50 MHz |
| PA6 | SPI1_MISO | DAC_MISO | Pin 7 (SDO) | Readback del DAC (opcional — conectar para debug) |
| PA7 | SPI1_MOSI | DAC_MOSI | Pin 6 (SDI) | Datos hacia el DAC (MSB first, 16 bits) |
| PB6 | GPIO_OUT | DAC_CSn | Pin 9 (~SYNC) | Chip Select activo LOW — gestión manual en firmware |
| PB7 | GPIO_OUT | DAC_LDAC | Pin 10 (~LDAC) | Update latch — pulso LOW actualiza los 4 canales simultáneamente |

**Asignación de los 4 canales DAC8564:**

| Canal | Net | Va a | Voltaje | Para qué |
|---|---|---|---|---|
| DAC_A | CV_VCF | AS3320 pin VIN via OPA2134 | 0–5V | Frecuencia de corte del filtro |
| DAC_B | CV_VCA | THAT2180 pin VG via OPA2134 | 0–5V | Nivel del amplificador |
| DAC_C | CV_PITCH | Spare / futuro Pitch CV | 0–5V | Ajuste de tono (expansión) |
| DAC_D | CV_AUX | Libre | 0–5V | Libre para expansión |

**En CubeMX:**
- `SPI1 → Mode: Full-Duplex Master`
- Data Size: `8 bits` (se envían 3 bytes en secuencia con CS manual)
- Clock Polarity (CPOL): `Low`
- Clock Phase (CPHA): `2 Edge` (CPOL=0 CPHA=1 = Modo SPI 1)
- NSS: `Software` (CS controlado manualmente via PB6)
- Prescaler: Buscá que el baudrate sea ≤ 50 MHz (SPI1 corre desde PCLK2 ≤ 240 MHz → Prescaler /8 = 30 MHz)

---

## 8. SPI2 — FLASH DE PRESETS → W25Q128

Almacena todos los presets del sintetizador (tiene espacio para miles de parches).

| Pin STM32 | Función CubeMX | Net KiCad | Va a W25Q128 pin | Descripción |
|---|---|---|---|---|
| PB13 | SPI2_SCK | FLASH_SCK | Pin 6 (CLK) | Clock SPI |
| PB5 | SPI2_MOSI | FLASH_MOSI | Pin 5 (DI) | Datos hacia la Flash |
| PB4 | SPI2_MISO | FLASH_MISO | Pin 2 (DO) | Datos desde la Flash |
| PB14 | GPIO_OUT | FLASH_CSn | Pin 1 (~CS) | Chip Select activo LOW + pull-up 100 kΩ a +3V3_DIG |
| PB15 | GPIO_OUT | FLASH_WPn | Pin 3 (~WP) | Write Protect — conectar a +3V3_DIG para nunca proteger, o GPIO para controlarlo |

> **PB4 (SPI2_MISO)** — Este pin es por defecto NJTRST (JTAG). Si usas SWD (no JTAG
> completo) queda libre para SPI2_MISO. En CubeMX esto se configura automáticamente
> al seleccionar SWD en lugar de JTAG.

**En CubeMX:**
- `SPI2 → Mode: Full-Duplex Master`
- Data Size: `8 bits`
- CPOL: `Low`, CPHA: `1 Edge` (Modo SPI 0 — W25Q128 compatible)
- NSS: `Software`
- Prescaler: Para 30 MHz (SPI2 desde PCLK1 ≤ 120 MHz → Prescaler /4 = 30 MHz)

---

## 9. I2C1 — BUS COMPARTIDO

Un solo bus I2C controla cuatro ICs diferentes, identificados por su dirección.
Este bus **debe tener pull-ups** de 4.7 kΩ a +3V3_DIG (un par en el PCB, no por IC).

| Pin STM32 | Función CubeMX | Net KiCad | Descripción |
|---|---|---|---|
| PB8 | I2C1_SCL | I2C_SCL | Clock I2C — pull-up 4.7 kΩ a +3V3_DIG en el PCB |
| PB9 | I2C1_SDA | I2C_SDA | Datos I2C — pull-up 4.7 kΩ a +3V3_DIG en el PCB |

**Dispositivos en el bus I2C1:**

| IC | Dirección I2C | Para qué |
|---|---|---|
| MCP23017 | 0x20 (A2=A1=A0=GND) | Leer los 16 Cherry MX switches + botones de función |
| SSD1306 | 0x3C (SA0=GND) | Display OLED 128×64 — menús y parámetros |
| PCA9685 | 0x40 (A5=A4=A3=A2=A1=A0=GND) | Driver LEDs PWM de los 16 step switches |
| PCM5242 | 0x4C (ADR1=L, ADR0=L) | Configuración del DAC de audio (volumen digital, formato, etc.) |

**GPIOs de control del bus I2C:**

| Pin STM32 | Función CubeMX | Net KiCad | Descripción |
|---|---|---|---|
| PC0 | GPIO_Input + EXTI | MCP23017_INT | Interrupción del MCP23017 INT_A (activo LOW) — avisa cuando se presiona un switch |
| PC1 | GPIO_Output | PCA9685_OE | Output Enable del PCA9685 — LOW activa los LEDs. Pull-down 10 kΩ a GND en PCB |
| PC3 | GPIO_Output | PCM5242_PDN | Power Down del PCM5242 — HIGH = operación normal. Pull-up 10 kΩ a +3V3_DIG |

**En CubeMX:**
- `I2C1 → Mode: I2C`
- Speed Mode: `Fast Mode 400 kHz`
- PC0: `GPIO_EXTI0 → Rising/Falling edge → Pull-up` (la INT del MCP23017 es open-drain)

---

## 10. ADC — POTENCIÓMETROS Y CV INPUT

### 10.1 ADC1 — Potenciómetros físicos (2 pots)

| Pin STM32 | Función CubeMX | Net KiCad | Pot físico | Descripción |
|---|---|---|---|---|
| PA0 | ADC1_INP16 | POT_VOLUME | RV3 — 10 kΩ lineal | Master Volume — wiper entre GND y +3V3_DIG |
| PA1 | ADC1_INP17 | POT_CUTOFF | RV4 — 10 kΩ lineal | Filter Cutoff backup — wiper entre GND y +3V3_DIG |

> **Importante:** El extremo VCC de los pots va a **+3V3_DIG**, **nunca** a +5V.
> El ADC del STM32H743 tolera máximo VDDA (3.3V). +5V destruye el ADC.
> Añadir 10 nF entre el wiper y AGND para filtrar ruido de aliasing.

### 10.2 ADC3 — CV Input externo (volumen de entrada externa 0–5V)

| Pin STM32 | Función CubeMX | Net KiCad | Descripción |
|---|---|---|---|
| PC2 | ADC3_INP0 | CV_IN | Entrada de Control Voltage externo (0–5V) reducida a 0–3.3V via divisor 3:5 |

**Divisor de tensión CV_IN:**
```
CV externo (0–5V) ──[30kΩ]──┬── PC2 (ADC3_INP0) — máx 3.3V
                             │
                           [20kΩ]
                             │
                           AGND

Fórmula: Vout = Vcv × 20/(30+20) = Vcv × 0.4
Con Vcv = 5V → Vout = 2.0V  ✓ (por debajo del límite 3.3V)
Con Vcv = 3.3V → Vout = 1.32V
```

**En CubeMX:**
- `ADC1 → IN16 + IN17 → Independent mode`
- `ADC3 → IN0 → Independent mode`
- Resolución: 16 bits
- Habilitar DMA para ADC1 si quieres lectura continua de los pots sin bloquear el CPU

---

## 11. ENCODERS ROTATIVOS EC11 — HARDWARE TIM (Encoders 1–4)

Los primeros 4 encoders usan timers dedicados en **modo quadrature**.
El STM32 cuenta automáticamente los pulsos sin interrupciones — solo leer el contador.

| Encoder | Pin A | Pin B | Timer | Canal A | Canal B | Net A | Net B |
|---|---|---|---|---|---|---|---|
| ENC1 | PA15 | PB3 | TIM2 | TIM2_CH1 | TIM2_CH2 | ENC1_A | ENC1_B |
| ENC2 | PC6 | PC7 | TIM3 | TIM3_CH1 | TIM3_CH2 | ENC2_A | ENC2_B |
| ENC3 | PD12 | PD13 | TIM4 | TIM4_CH1 | TIM4_CH2 | ENC3_A | ENC3_B |
| ENC4 | PE9 | PE11 | TIM1 | TIM1_CH1 | TIM1_CH2 | ENC4_A | ENC4_B |

**Circuito por encoder (repetir ×4):**
```
EC11 Pin A ──[10 kΩ pull-up a +3V3_DIG]── TIMx_CH1 ──[100 nF a GND]
EC11 Pin B ──[10 kΩ pull-up a +3V3_DIG]── TIMx_CH2 ──[100 nF a GND]
EC11 COM   ──► DGND
```

**En CubeMX (repetir para TIM1, TIM2, TIM3, TIM4):**
- `TIMx → Combined Channels → Encoder Mode`
- Encoder Mode: `Encoder Mode TI1 and TI2`
- Counter Period: `65535` (16 bits, máximo)
- En código: leer `__HAL_TIM_GET_COUNTER(&htimX)` y calcular delta respecto al valor anterior

> **Por qué 4 flancos por muesca:** El EC11 de 20 pulsos/vuelta genera 4 flancos por
> paso mecánico en modo quadrature (TI1 y TI2). El contador avanza/retrocede de 4 en 4.
> Dividir el delta por 4 para obtener el número real de muescas giradas.

---

## 12. ENCODERS ROTATIVOS EC11 — SOFTWARE GPIO (Encoders 5–8)

Los encoders 5–8 no tienen timer propio disponible sin conflictos.
Se leen por **EXTI en el canal A** y se sondea el canal B en la ISR para determinar dirección.
Funciona perfectamente para uso en UI — latencia < 1 ms.

| Encoder | Pin A (EXTI) | Pin B (GPIO poll) | Net A | Net B |
|---|---|---|---|---|
| ENC5 | PC8 | PC9 | ENC5_A | ENC5_B |
| ENC6 | PC10 | PC11 | ENC6_A | ENC6_B |
| ENC7 | PC12 | PD2 | ENC7_A | ENC7_B |
| ENC8 | PD3 | PD4 | ENC8_A | ENC8_B |

**En CubeMX:**
- Pines A (PC8, PC10, PC12, PD3): `GPIO_EXTI → Both Edges → Pull-up`
- Pines B (PC9, PC11, PD2, PD4): `GPIO_Input → Pull-up`

**Lógica en firmware (ISR del EXTI):**
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    // En la interrupción de flanco del pin A:
    uint8_t pinA = HAL_GPIO_ReadPin(ENCx_A_PORT, ENCx_A_PIN);
    uint8_t pinB = HAL_GPIO_ReadPin(ENCx_B_PORT, ENCx_B_PIN);
    if (pinA == pinB)  encoder_count--;   // giro anti-horario
    else               encoder_count++;   // giro horario
}
```



## 13. PUSH DE ENCODERS — BOTONES

Cada encoder EC11 tiene un botón integrado (push al presionar el eje).
Son 8 botones independientes, leídos directamente por GPIO del STM32.

| Encoder | Pin GPIO | Net KiCad | Descripción |
|---|---|---|---|
| BTN_ENC1 | PD5 | BTN_ENC1 | Push encoder 1 |
| BTN_ENC2 | PD6 | BTN_ENC2 | Push encoder 2 |
| BTN_ENC3 | PD7 | BTN_ENC3 | Push encoder 3 |
| BTN_ENC4 | PD8 | BTN_ENC4 | Push encoder 4 |
| BTN_ENC5 | PD9 | BTN_ENC5 | Push encoder 5 |
| BTN_ENC6 | PD10 | BTN_ENC6 | Push encoder 6 |
| BTN_ENC7 | PD11 | BTN_ENC7 | Push encoder 7 |
| BTN_ENC8 | PE0 | BTN_ENC8 | Push encoder 8 |

**Circuito por botón (repetir ×8):**

EC11 Push pin ──[10 kΩ pull-up a +3V3_DIG]── GPIO_Input + [100 nF a GND]
EC11 Push COM ──► DGND


**En CubeMX:**
- Todos: `GPIO_Input → Pull-up` (el botón lleva el pin a GND al presionar)
- Opcional: `EXTI → Falling Edge` para respuesta por interrupción en lugar de polling

> **Los 16 Cherry MX switches y los botones de función** van a través del **MCP23017**
> por I2C — no consumen pines del STM32. Solo el pin PC0 (INT) es necesario.



## 14. RESUMEN DE PINES USADOS / LIBRES

### Mapa completo por puerto

| Puerto PA | Función asignada |
|---|---|
| PA0 | ADC1_INP16 — POT_VOLUME |
| PA1 | ADC1_INP17 — POT_CUTOFF |
| PA2 | **LIBRE** |
| PA3 | **LIBRE** |
| PA4 | **LIBRE** |
| PA5 | SPI1_SCK — DAC_SCK |
| PA6 | SPI1_MISO — DAC_MISO |
| PA7 | SPI1_MOSI — DAC_MOSI |
| PA8 | **LIBRE** |
| PA9 | USART1_TX — MIDI_TX |
| PA10 | USART1_RX — MIDI_RX |
| PA11 | USB_OTG_FS_DM — USB_DM |
| PA12 | USB_OTG_FS_DP — USB_DP |
| PA13 | SYS_SWDIO — SWDIO |
| PA14 | SYS_SWCLK — SWDCK |
| PA15 | TIM2_CH1 — ENC1_A |

| Puerto PB | Función asignada |
|---|---|
| PB0 | **LIBRE** |
| PB1 | **LIBRE** |
| PB2 | **LIBRE** |
| PB3 | TIM2_CH2 — ENC1_B |
| PB4 | SPI2_MISO — FLASH_MISO |
| PB5 | SPI2_MOSI — FLASH_MOSI |
| PB6 | GPIO_OUT — DAC_CSn |
| PB7 | GPIO_OUT — DAC_LDAC |
| PB8 | I2C1_SCL — I2C_SCL |
| PB9 | I2C1_SDA — I2C_SDA |
| PB10 | **LIBRE** |
| PB11 | **LIBRE** |
| PB12 | **LIBRE** |
| PB13 | SPI2_SCK — FLASH_SCK |
| PB14 | GPIO_OUT — FLASH_CSn |
| PB15 | GPIO_OUT — FLASH_WPn |

| Puerto PC | Función asignada |
|---|---|
| PC0 | GPIO_EXTI — MCP23017_INT |
| PC1 | GPIO_OUT — PCA9685_OE |
| PC2 | ADC3_INP0 — CV_IN |
| PC3 | GPIO_OUT — PCM5242_PDN |
| PC4 | **LIBRE** |
| PC5 | **LIBRE** |
| PC6 | TIM3_CH1 — ENC2_A |
| PC7 | TIM3_CH2 — ENC2_B |
| PC8 | GPIO_EXTI — ENC5_A |
| PC9 | GPIO_Input — ENC5_B |
| PC10 | GPIO_EXTI — ENC6_A |
| PC11 | GPIO_Input — ENC6_B |
| PC12 | GPIO_EXTI — ENC7_A |
| PC13 | **LIBRE** (cuidado: RTC/WKUP — usar con reserva) |
| PC14 | **LIBRE** (cuidado: OSC32_IN — usar con reserva) |
| PC15 | **LIBRE** (cuidado: OSC32_OUT — usar con reserva) |

| Puerto PD | Función asignada |
|---|---|
| PD0 | **LIBRE** |
| PD1 | **LIBRE** |
| PD2 | GPIO_Input — ENC7_B |
| PD3 | GPIO_EXTI — ENC8_A |
| PD4 | GPIO_Input — ENC8_B |
| PD5 | GPIO_Input — BTN_ENC1 |
| PD6 | GPIO_Input — BTN_ENC2 |
| PD7 | GPIO_Input — BTN_ENC3 |
| PD8 | GPIO_Input — BTN_ENC4 |
| PD9 | GPIO_Input — BTN_ENC5 |
| PD10 | GPIO_Input — BTN_ENC6 |
| PD11 | GPIO_Input — BTN_ENC7 |
| PD12 | TIM4_CH1 — ENC3_A |
| PD13 | TIM4_CH2 — ENC3_B |
| PD14 | **LIBRE** |
| PD15 | **LIBRE** |

| Puerto PE | Función asignada |
|---|---|
| PE0 | GPIO_Input — BTN_ENC8 |
| PE1 | **LIBRE** |
| PE2 | SAI1_MCLK_A — I2S_MCLK |
| PE3 | SAI1_SD_B — I2S_SD |
| PE4 | SAI1_FS_A — I2S_LRCK |
| PE5 | SAI1_SCK_A — I2S_BCK |
| PE6 | **LIBRE** |
| PE7 | **LIBRE** |
| PE8 | **LIBRE** |
| PE9 | TIM1_CH1 — ENC4_A |
| PE10 | **LIBRE** |
| PE11 | TIM1_CH2 — ENC4_B |
| PE12 | **LIBRE** |
| PE13 | **LIBRE** |
| PE14 | **LIBRE** |
| PE15 | **LIBRE** |

| Puerto PH | Función asignada |
|---|---|
| PH0 | OSC_IN — CRISTAL 25MHz |
| PH1 | OSC_OUT — CRISTAL 25MHz |


## 15. CHECKLIST CUBEMX — PASO A PASO

Sigue este orden exacto en STM32CubeMX para evitar conflictos de pines:

### Paso 1 — Crear el proyecto

File → New Project → Board Selector
Buscar: STM32H743VIT6
Accept default


### Paso 2 — Configurar el oscilador

Pinout → RCC
  HSE: Crystal/Ceramic Resonator
  LSE: Disable (no usamos RTC)


### Paso 3 — SWD

Pinout → SYS
  Debug: Serial Wire
  (PB3 como SWO: desactivar — lo necesitamos para ENC1)


### Paso 4 — USB OTG FS

Pinout → USB_OTG_FS
  Mode: Device Only
  Activate_VBUS: No


### Paso 5 — SAI1 (audio)

Pinout → SAI1
  Block A: Master TX
  Block B: Synchronous Slave TX
  Audio Frequency: 48000 Hz
  Frame Length: 64 (32 bits × 2 canales)
  Habilitar MCLK Output en Block A


### Paso 6 — SPI1 (DAC8564)

Pinout → SPI1
  Mode: Full-Duplex Master
  Hardware NSS: Disable
  CPOL: Low  /  CPHA: 2 Edge
  Data Size: 8 Bits
  Prescaler: /8 (verifica que baudrate ≤ 50 MHz)


### Paso 7 — SPI2 (Flash)

Pinout → SPI2
  Mode: Full-Duplex Master
  Hardware NSS: Disable
  CPOL: Low  /  CPHA: 1 Edge
  Data Size: 8 Bits


### Paso 8 — I2C1 (bus compartido)

Pinout → I2C1
  Mode: I2C
  Speed Mode: Fast Mode 400 kHz


### Paso 9 — USART1 (MIDI DIN)

Pinout → USART1
  Mode: Asynchronous
  Baud Rate: 31250
  Word Length: 8 Bits
  Parity: None  /  Stop Bits: 1


### Paso 10 — ADC

Pinout → ADC1
  IN16: Single-ended  (PA0 — pot volume)
  IN17: Single-ended  (PA1 — pot cutoff)
  Resolution: 16 bits

Pinout → ADC3
  IN0: Single-ended   (PC2 — CV input)
  Resolution: 16 bits


### Paso 11 — Timers de encoders

Pinout → TIM1: Combined Channels → Encoder Mode
Pinout → TIM2: Combined Channels → Encoder Mode
Pinout → TIM3: Combined Channels → Encoder Mode
Pinout → TIM4: Combined Channels → Encoder Mode
  (Para todos: Encoder Mode TI1 and TI2, Period 65535)


### Paso 12 — GPIOs
Configurar manualmente en el mapa de pines:

| Pin | Tipo | Label |
|---|---|---|
| PB6 | GPIO_Output | DAC_CSn |
| PB7 | GPIO_Output | DAC_LDAC |
| PB14 | GPIO_Output | FLASH_CSn |
| PB15 | GPIO_Output | FLASH_WPn |
| PC0 | GPIO_EXTI0 (Both edges, Pull-up) | MCP23017_INT |
| PC1 | GPIO_Output (init HIGH) | PCA9685_OE |
| PC3 | GPIO_Output (init HIGH) | PCM5242_PDN |
| PC8 | GPIO_EXTI8 (Both edges, Pull-up) | ENC5_A |
| PC9 | GPIO_Input (Pull-up) | ENC5_B |
| PC10 | GPIO_EXTI10 (Both edges, Pull-up) | ENC6_A |
| PC11 | GPIO_Input (Pull-up) | ENC6_B |
| PC12 | GPIO_EXTI12 (Both edges, Pull-up) | ENC7_A |
| PD2 | GPIO_Input (Pull-up) | ENC7_B |
| PD3 | GPIO_EXTI3 (Both edges, Pull-up) | ENC8_A |
| PD4 | GPIO_Input (Pull-up) | ENC8_B |
| PD5–PD11 | GPIO_Input (Pull-up) | BTN_ENC1–7 |
| PE0 | GPIO_Input (Pull-up) | BTN_ENC8 |

### Paso 13 — Configurar el Clock Tree

Clock Configuration tab:
  Input Frequency: 25 MHz (HSE)
  
  PLL1:
    DIVM1 = 5   → 5 MHz
    MULN1 = 192 → 960 MHz VCO
    DIVP1 = 2   → 480 MHz (SYSCLK)
    DIVQ1 = 4   → 240 MHz (disponible)
    DIVR1 = 2   → 480 MHz (disponible)
  
  PLL3:
    DIVM3 = 5   → 5 MHz
    MULN3 = 192 → 960 MHz VCO
    DIVP3 = 20  → 48 MHz exacto (USB OTG FS) ✓
    DIVQ3 = 20  → 48 MHz exacto (SAI1 kernel clock) ✓


### Paso 14 — Habilitar DMA

DMA:
  SAI1_A → DMA1 Stream 0 → Memory to Peripheral → Circular → Word
  ADC1   → DMA1 Stream 1 → Peripheral to Memory → Circular → HalfWord


### Paso 15 — Generar código

Project Manager:
  Project Name: HybridSynth_v1
  Toolchain: STM32CubeIDE
  Generate peripheral initialization as pair of .c/.h files: ✓
Generate Code




## 16. CONFLICTOS Y RESOLUCIONES

| Conflicto | Problema | Resolución elegida |
|---|---|---|
| PA0/PA1 vs TIM5 encoder | PA0/PA1 son ADC para los pots; TIM5 encoder también usa PA0/PA1 | **No usar TIM5 en PA0/PA1.** ENC4 usa TIM1 en PE9/PE11 que están libres |
| PB3 como SWO vs TIM2_CH2 (ENC1_B) | PB3 puede ser SWO (debug ITM) o TIM2_CH2 para encoder | **Prioridad al encoder.** SWO desactivado. Para debug usar semihosting o UART |
| PB4 como NJTRST vs SPI2_MISO | PB4 es NJTRST en modo JTAG completo | **Usar modo SWD** (no JTAG completo) → PB4 queda libre para SPI2_MISO |
| PA13/PA14 (SWD) permanentes | SWD ocupa PA13/PA14 siempre | Aceptado — son los únicos pines de programación. No reasignar nunca |
| PC13–PC15 uso restringido | Estos pines tienen limitaciones de corriente y se usan para RTC/tamper | **No usarlos** como GPIO de propósito general — dejados libres |
| TIM5 con 8 encoders | Solo hay 4 timers con modo encoder disponibles sin conflictos (TIM1-4) | **ENC5–8 por software GPIO+EXTI** — perfectamente funcional para uso de UI |
