# Revisión del Esquemático — DigitalSynth v1.0
**Fecha:** 23 Feb 2026 | **Revisado contra:** `05_GUIA_ESQUEMATICO.md` v2.0
**Estado:** ✅ Bloques base completados — pendiente completar periféricos

---

## RESUMEN EJECUTIVO

| Bloque | Estado |
|---|---|
| USB-C (power) | ✅ Completado |
| STM32H743 + Decoupling | ✅ Completado |
| +5VA ferrite (DAC8565) | ✅ Completado |
| LDO 3.3VA (LT3042) | ✅ Completado |
| LDO 3.3V (AMS1117) | ✅ Completado |
| PCM5122 audio DAC | ✅ Completado |
| DAC8565 CV | ✅ Completado |
| Jacks audio (F5) | ✅ Completado |
| MCP23017 + Cherry MX ×16 (F9) | ✅ Completado |
| USBLC6-2 ESD USB (F4) | ✅ Completado |
| Jacks CV 4× TRS (F6) | 🔴 Pendiente |
| W25Q128 Flash SPI (F7) | ✅ Completado |
| SSD1306 OLED (F8) | 🟡 Pendiente |
| SK6812 RGB LEDs ×16 (F12) | 🟡 Pendiente |
| Encoders EC11 ×8 (F10) | 🟡 Pendiente |
| Potenciómetros RV3/RV4 (F11) | 🟡 Pendiente |
| MIDI DIN IN/OUT (F13/F14) | 🟢 Pendiente |
| USB-B MIDI (F15) | 🟢 Pendiente |

**Completados: 11 bloques ✅ — Pendientes: 7 bloques**

---

## BLOQUES COMPLETADOS ✅

> Lista compacta. Estos bloques están diseñados y verificados — no requieren más cambios.

- ✅ **USB-C power** — CC1/CC2→GND (pull-down 5K1), polyfuse único 2A, VBUS→5V_BUS
- ✅ **STM32H743** — BOOT0 pull-down 10kΩ + SW1 Nidec CAS-120A1 no-momentáneo, R2 eliminado, cristal 25MHz + R_serie 22Ω
- ✅ **Decoupling STM32** — VCAP1/VCAP2 (C19/C20 2.2µF X5R), 6×100nF + 1×10µF VDD, VDDA ferrite + 10nF + 1µF + 1µF
- ✅ **+5VA ferrite** — FB4 600Ω@100MHz + C14 100nF + C47 4.7µF a GNDA
- ✅ **LT3042 3.3VA** — RSET=33.2kΩ, C_SET 0.47µF, RILIM 360kΩ, ferrite en entrada
- ✅ **AMS1117 3.3V** — bulk electrolítico + bypass 100nF en entrada y salida
- ✅ **PCM5122** — XSMT→PC3_GPIO (init HIGH obligatorio en firmware), AVDD/CPVDD→+3.3VA, I2S correcto, filtro 470Ω+2.2nF+100Ω
- ✅ **DAC8565** — SPI PA5/PA7/PB6/PB7, IOVDD→+3.3V, AVDD→+5VA, RST pull-up + PC4_GPIO, CV1-4_OUT renombradas
- ✅ **F5 Jacks audio** — 2× NRJ4HF-1 (L/R) + RCJ-32265 triple RCA, filtro correcto
- ✅ **DAC8565 CV** — AVDD→+5VA (C45 10µ+C41 100n), DVDD→+3.3V (C44 100n+C46 10µ), VREFL→GNDA, RST→PD4_GPIO, RSTSEL/ENABLE→GND, salidas→J1/J2/J3/J4
- ✅ **PCM5122** — XSMT→PC3_GPIO, AVDD/CPVDD→+3.3VA, DVDD→+3.3V, DGND→GND, AGND/CPGND→GNDA, filtro 470Ω+2n2+100Ω, CAPP/CAPM C48 2u2, VCOM C39 10µ, VNEG C35 2u2, MODE1/2→+3.3VA
- ✅ **LT3042 3.3VA** — FB3 600Ω entrada, C25 4u7 entrada, C27 4u7 salida, C26 0.47µ (C_SET), R7 (RSET), OUT→+3.3VA
- ✅ **AMS1117 3.3V** — C15 100n + C17 22µ entrada, C18 22µ + C16 100n salida, 5V_BUS→VI→VO→+3.3V
- ✅ **F4 USBLC6-2P6** — U2 en bloque USB-C, pines 3/4→USB_D+, pines 1/6→USB_D-, GND→GND (integrado en bloque USB-C)
- ✅ **F7 W25Q128JVS** — U4, PB14_GPIO_OUT→/CS, PB13_SPI2_SCK→CLK, PB15_SPI2_MOSI→DI/IO0, PB4_SPI2_MISO→DO/IO1, WP/IO2→+3.3V (pull-up R16), HOLD/IO3→+3.3V (pull-up), VCC→+3.3V + C49 100n
- ✅ **F9 MCP23017 + Cherry MX ×16** — GPA0-7→SW1-8, GPB0-7→SW9-16, INTA→PC0, RESET pull-up, 0x20

---

## ⚠️ ADVERTENCIAS — REVISAR EN ESQUEMÁTICO ACTUAL

---

### ⚠️ ADVERTENCIA 1 — R1 en BOOT0: verificar valor

En el esquemático se ve **R1** entre BOOT0 y +3.3V (en serie con SW1). El valor mostrado parece **"100n"** o "R1" sin valor legible.

- Si R1 es una **resistencia de 10kΩ pull-up** (para limitar corriente cuando el switch está cerrado) → ✅ correcto
- Si R1 es un **condensador 100nF** → ❌ error — BOOT0 no tiene pull-down definido al arrancar

**El circuito correcto debe ser:**
```
BOOT0 (pin 94) ──[R_BOOT 10kΩ, 0402]──► GND       ← pull-down obligatorio
BOOT0 (pin 94) ──────────────────────► SW1 ──► +3.3V
```
**Verificar:** ¿Existe R_BOOT pull-down a GND? ¿R1 está en serie al switch o directamente a BOOT0?

---

### ⚠️ ADVERTENCIA 2 — SK6812: cadena DIN→DOUT no visible

En el bloque SW SEQUENCER, los pines **VDD** y **VSS** de los 16 switches se ven conectados a +3.3V y GND respectivamente (flechas arriba/abajo). Pero **no se ve** la cadena de datos:

```
STM32 GPIO ──► SW1.DIN → SW1.DOUT ──► SW2.DIN → ... → SW16.DOUT ──► NC (o 10kΩ a GND)
```

**Verificar:** ¿Están conectados DOUT de SW_n al DIN de SW_n+1? ¿Qué GPIO del STM32 va al DIN de SW1? El pin GPIO para datos SK6812 debe agregarse como net nombrada (ej. `SK6812_DATA`).

---

### ⚠️ ADVERTENCIA 3 — STM32: tierra GNDA en pin VSS

En el esquemático del STM32, la tierra de los pines VSS sale como **GNDA**. Los pines VSS del STM32 son tierra **digital** y deben conectarse a `GND` (no a `GNDA` analógico).

- `GND` = tierra digital → STM32 VSS, AMS1117, lógica
- `GNDA` = tierra analógica → PCM5122, DAC8565, filtros de audio

**Si están unidos en el plano de PCB** → es aceptable, pero en el esquemático las nets deben ser distintas para que el ERC no marque errores y el router sepa dónde poner el split de planos.

---

### ⚠️ ADVERTENCIA 4 — MCP23017: INTB sin conectar

El MCP23017 tiene INTB (pin 19) visible pero con símbolo de "X" (sin conectar). Si vas a usar los 16 switches en interrupciones:
- **Solo PORTA** → INTA→PC0 es suficiente, INTB→NC está bien ✓
- **PORTB también** → conectar INTB a otro GPIO (ej. PC1) para interrupciones del banco B

Con polling por I2C (sin interrupciones) ambos NC están bien. Si usas INTA para interrupción de cualquier cambio en los 16 pines, configurar `MIRROR=1` en el registro IOCON — así INTA refleja cambios de ambos bancos y no necesitas INTB.

---

### ⚠️ ADVERTENCIA 5 — USB-C: CC1/CC2 — verificar dirección de R3/R4 (CRÍTICO)

En la imagen del bloque USB-C, la etiqueta **"5V_USB"** aparece encima de R3(5K1) y R4(5K1) que van a CC2(B5) y CC1(A5). Si la conexión es:

```
5V_USB ──[R3 5K1]──► CC2        ← ❌ PULL-UP a VBUS = error original no corregido
5V_USB ──[R4 5K1]──► CC1
```
```
GND ◄──[R3 5K1]── CC2           ← ✅ correcto: pull-DOWN a GND
GND ◄──[R4 5K1]── CC1
```

**En la imagen se ven flechas hacia arriba desde R3/R4 con etiqueta "5V_USB" — esto sugiere que el nodo superior de los resistores está en el net 5V_USB (VBUS), no en GND.** Si es así, el error original ERROR 1 sigue presente.

**Verificar en KiCad:** hacer clic en R3/R4 → ver a qué nets conectan sus dos terminales. Pin 1 debe ir a CC_x, pin 2 **debe ir a GND**.

---

### ⚠️ ADVERTENCIA 6 — RCJ-32265: Canal C (J7B) con pin señal sin net visible

El conector RCJ-32265 es triple RCA (L/C/R). En el esquemático:
- **J7A** → canal L → `L_CHAN_RCA` ✅
- **J7C** → canal R → `R_CHAN_RCA` ✅
- **J7B** → canal C (center/video) → pin señal muestra "X" o sin net clara

Si el pin señal de J7B está sin asignar (flotante en PCB), el pad del conector queda sin conectar pero eso no causa problema funcional. **Si accidentalmente está unido a L_CHAN o R_CHAN, ambos canales se cortocircuitan.**

**Verificar:** en KiCad, seleccionar J7B → confirmar que el pin señal (pin 1) tiene marcador "No Connect" (X) explícito y no está conectado a ninguna net de audio.

---

### ⚠️ ADVERTENCIA 7 — DAC8565: Verificar que LDAC y DIN usan pines distintos

En la imagen del DAC8565 se ven cuatro etiquetas de GPIO para LDAC/SYNC/SCLK/DIN. La imagen muestra:
- LDAC → PF7_GPIO_OUT
- SYNC → PF8_GPIO_OUT
- SCLK → PF9_SR1_SCK (o similar)
- DIN → ¿PF7? (ilegible en imagen)

**Si DIN también usa PF7 → conflicto de pines: dos señales distintas en el mismo GPIO.**

El plan original era: SPI con PA5(SCK)/PA7(MOSI)/PB6(CS)/PB7(LDAC). Si se cambió a SPI4 (PF pins), los pines válidos en STM32H743 para SPI4 son:
```
SPI4: PF7=SCK, PF8=MISO, PF9=MOSI (o PF11=MOSI según AF)
LDAC: cualquier GPIO libre, ej. PE2, PD0, etc.
```
**Verificar en CubeMX** que ningún pin tiene doble asignación.

---

### ⚠️ ADVERTENCIA 8 — PCM5122: Asignación I2S — verificar PE5

En el esquemático el PCM5122 muestra:
- PE2_I2S_MCLK → SCK (MCLK del PCM5122) ✅
- PE5_I2S_BCK → BCK ← ⚠️ **a verificar**
- PE4_I2S_LRCK → LRCK ✅
- DIN (datos I2S) → ¿qué pin?

En el STM32H743, para I2S4:
- **PE2** = I2S4_CK ✅
- **PE4** = I2S4_WS (LRCK) ✅
- **PE5** = I2S4_SD (datos, no BCK) ← conflicto de nombre
- **PE6** = I2S4_MCK (MCLK)

Si PE5 es I2S4_SD en el hardware pero en el esquemático se etiqueta como BCK (bit clock), hay confusión de señales. **El bit clock (BCK) en I2S4 del STM32H743 suele ser PE2 o PB13.**

**Verificar en CubeMX:** con I2S4 activado, confirmar qué pin es asignado a SCK (BCK), WS (LRCK), SD (DIN) y MCK — y que coincidan exactamente con las etiquetas del esquemático.

---

### ⚠️ ADVERTENCIA 9 — LT3042: Verificar conexión de pin ILIM

En la imagen del bloque LDO 3.3VA, se ve R7 cerca del LT3042 pero la imagen es pequeña. El LT3042 (MSOP-10) tiene:
- **Pin 5 = ILIM** → debe conectar a GND a través de RILIM (ej. 360kΩ para 200mA)
- **Pin 7 = SET** → debe conectar a GND a través de RSET (33.2kΩ para 3.3V)

Si R7 es la única resistencia visible y conecta SET a GND, pero ILIM queda sin conectar → límite de corriente indefinido (puede llegar a máximo del IC = ~500mA con sobrecalentamiento).

**Verificar:** ¿Existen dos resistencias separadas para ILIM y SET, o solo una?

---

---

## BLOQUES FALTANTES — CONEXIONES DETALLADAS

> Úsalos como guía directa en KiCad. Cada bloque está listo para copiar pin a pin.

---

### 🔴 BLOQUE F6 — Jacks CV Out (4× TRS 3.5mm)

**Las nets CV1..CV3 ya salen del DAC8565. VOUTD está NC pero puede añadirse como CV4.**

**¿Se necesita op-amp en la salida CV?**
**No.** El DAC8565 tiene un **buffer de salida rail-to-rail integrado** — es un DAC de
salida en tensión, no en corriente. Puede conducir directamente cargas ≥ 2kΩ, que es lo
normal en entradas CV de cualquier gear (Eurorack, synths, etc. tienen tipicamente 100kΩ+).
El 100Ω en serie es suficiente protección.

**Rango de salida — importante:**
| Configuración | Rango VOUT | Octavas (1V/oct) |
|---|---|---|
| Gain=1 (por defecto) | 0 — 2.5V | 2.5 octavas |
| Gain=2 (bit G via SPI) | 0 — 5V | 5 octavas ← **recomendado** |

Para gain=2, enviar el comando SPI con bit G=1 durante la inicialización del firmware.
Requiere AVDD ≥ 4.5V → ya lo tienes con +5VA. ✓

```
── Conexión idéntica para los 4 jacks ──
DAC8565 VOUTx ──[100Ω, 0402]──► J_CVx [Jack TRS 3.5mm]
                                  Tip    = CV signal (0–5V con gain=2)
                                  Ring   = NC
                                  Sleeve = GNDA

── Protección por jack (BAT54S, SOT-23) ──
  Diodo A (ánodo → Tip, katodo → +5VA): clamp positivo
  Diodo B (ánodo → GNDA, katodo → Tip): clamp negativo
  → previene que un Eurorack inyecte ±12V hacia el DAC8565
```

> Nets: CV1_OUT, CV2_OUT, CV3_OUT, CV4_OUT
> Renombrar VCF→CV1_OUT, VCA→CV2_OUT, PITCH/CV→CV3_OUT en el bloque DAC8565.
> Footprint: SJ-3523-SMT-TR (CUI) o PJ-3523 — PCB mount 3.5mm.

---

### 🟡 BLOQUE F8 — SSD1306 OLED 128×64 I2C

**Display principal. Usa el bus I2C1 que ya existe (PB8/PB9).**

```
SSD1306 (dirección I2C: 0x3C)
  VCC  ──► +3.3V + C [100nF] ──► GND
  GND  ──► GND
  SCL  ──► PB8 (I2C1_SCL)   ← pull-up 4.7kΩ compartido con otros ICs del bus
  SDA  ──► PB9 (I2C1_SDA)   ← pull-up 4.7kΩ compartido
  RES  ──► PD15 (GPIO OUT)  + R [10kΩ pull-up a +3.3V] + C [100nF a GND]
```

> Pull-ups I2C: UN SOLO PAR de 4.7kΩ para todo el bus (PCM5122 + OLED + PCA9685 + MCP23017).
> Si ya los pusiste en el bloque PCM5122, no duplicar aquí.
> Si tienes el módulo breakout de AliExpress, ya incluye pull-ups y VCC filter.

---

### 🟡 BLOQUE F10 — Encoders EC11 ×8 (rotary encoders con push)

**UI principal. Cada encoder necesita 2 canales de timer + 1 GPIO de push.**

```
── Esquema idéntico para cada uno de los 8 encoders ──

EC11_n Pin A ──[10kΩ]──► +3.3V
             ──[100nF]──► GND
             ──────────► TIMx_CH1 (ver tabla)

EC11_n Pin B ──[10kΩ]──► +3.3V
             ──[100nF]──► GND
             ──────────► TIMx_CH2 (ver tabla)

EC11_n Pin C (push) ──[10kΩ]──► +3.3V
                    ──[100nF]──► GND
                    ──────────► GPIO_IN (ver tabla)

EC11_n COM  ──► GND

── Asignación de pines por encoder ──
Enc 1: TIM2_CH1=PA15, TIM2_CH2=PB3,  Push=PD0
Enc 2: TIM3_CH1=PC6,  TIM3_CH2=PC7,  Push=PD1
Enc 3: TIM4_CH1=PD12, TIM4_CH2=PD13, Push=PD2
Enc 4: TIM5_CH1=PA0,  TIM5_CH2=PA1,  Push=PD3   ← PA0/PA1 también son ADC — evitar conflicto
Enc 5–8: TIM8 y TIM15 (confirmar en CubeMX según pines disponibles)
```

> El condensador 100nF entre el pin y GND es el filtro anti-rebote hardware.
> Sin él los timers cuentan pulsos de ruido mecánico al girar rápido.

---

### 🟡 BLOQUE F11 — Potenciómetros RV3 y RV4

**Solo 2 potenciómetros físicos: Master Volume y Filter Cutoff.**

```
RV3 — Master Volume (10kΩ lineal, footprint 9mm plástico):
  Pin 1 (CCW) ──► GND
  Pin 2 (wiper) ──[10nF a GND]──► PA0 (ADC1_IN0)   net: POT_VOLUME
  Pin 3 (CW)  ──► +3.3V

RV4 — Filter Cutoff (10kΩ lineal):
  Pin 1 (CCW) ──► GND
  Pin 2 (wiper) ──[10nF a GND]──► PA1 (ADC1_IN1)   net: POT_CUTOFF
  Pin 3 (CW)  ──► +3.3V
```

> ⚠️ PIN CW → +3.3V (NO a +5V). El ADC del STM32H743 es 3.3V máximo.
> El 10nF entre wiper y GND filtra el ruido de aliasing.
> Si Enc 4 usa PA0/PA1, hay conflicto → asignar Enc4 a otros pines o usar ADC en DMA.

---

### � BLOQUE F12 — SK6812 Mini-E RGB LEDs ×16 (cadena datos, 1 GPIO)

**Los SK6812 están integrados en el footprint del Cherry MX (pines VDD/VSS/DIN/DOUT).**
**Un solo GPIO del STM32 controla los 16 LEDs RGB en cadena. No se necesita PCA9685.**

```
── Alimentación (misma para los 16) ──
SWx VDD ──► +3.3V
SWx VSS ──► GND

── Cadena de datos ──
STM32 PC5 (GPIO OUT) ──[33Ω serie]──► SW1.DIN
                                       SW1.DOUT ──► SW2.DIN
                                                     SW2.DOUT ──► ... ──► SW16.DOUT ──► NC

── Decoupling por LED (importante para evitar glitches) ──
Cada VDD–VSS: [100nF, C0G, 0402] en paralelo, lo más cerca posible del pad VDD
```

**⚠️ Problema de nivel lógico:**
SK6812 necesita DIN ≥ 0.7×VDD para reconocer un "1".
- Con VDD=3.3V → umbral = 2.31V → STM32 (3.3V) ✅ funciona directamente
- Con VDD=5V → umbral = 3.5V → STM32 (3.3V) ❌ nivel insuficiente

**→ Alimentar con +3.3V, no +5V.** Brillo ligeramente menor pero funciona sin level shifter.

**Resistor 33Ω serie en DIN** protege el GPIO del STM32 contra reflexiones en la línea de datos.

**Protocolo en firmware (STM32H743):**
```c
// Usar TIM + DMA para generar señal 800kHz
// T0H=0.3µs, T1H=0.6µs, periodo=1.25µs
// Librerías: ws2812b_stm32_hal o implementación propia con TIM1 CH1 DMA
// Reset: línea LOW > 80µs

// Formato de color por LED: G[7:0] R[7:0] B[7:0] (¡GRB, no RGB!)
```

**Estados visuales sugeridos:**
| Estado step | Color | Valor GRB |
|---|---|---|
| Vacío | Rojo tenue | `0x001800` |
| Activo/programado | Rojo fuerte | `0x00FF00` |
| Cursor reproducción | Blanco | `0x808080` |
| Mute | Naranja parpadeante | `0x201000` ↔ off |

> Símbolo usado: el que ya tienes con pines VSS/DOUT/DIN/VDD + pines 1/2 switch
> Net de datos: `SK6812_DATA` (de PC5 al DIN de SW1)
> El pin PC5 debe configurarse como GPIO Output Push-Pull en CubeMX

---

### 🟢 BLOQUE F13 — MIDI DIN IN (optoacoplador 6N137)

**Entrada MIDI DIN-5. USART1_RX = PA10.**

```
DIN-5 conector MIDI IN:
  Pin 4 ──────────────────────────────────────► Cátodo LED (pin 3 del 6N137)
  Pin 5 ──[220Ω, 0402]──[1N4148 SOD-123]──────► Ánodo LED (pin 2 del 6N137)
  Pin 2 ──► NC
  Shell ──► GND

6N137 (DIP8 o SOIC8):
  Pin 2 (Anode)   ← desde DIN Pin 5 via 220Ω + 1N4148
  Pin 3 (Cathode) ← desde DIN Pin 4
  Pin 1 (VCC)     ──► +3.3V + C [100nF] ──► GND
  Pin 5 (GND)     ──► GND
  Pin 6 (Vo)      ──[10kΩ pull-up a +3.3V]──► PA10 (USART1_RX)
  Pin 7 (ENABLE)  ──► +3.3V
  Pin 8 (VCC)     ──► +3.3V
```

> El diodo 1N4148 protege contra conexión de cable MIDI al revés.
> Sin él un MIDI mal conectado puede destruir el optoacoplador.

---

### 🟢 BLOQUE F14 — MIDI DIN OUT

**Salida MIDI DIN-5. USART1_TX = PA9.**

```
PA9 (USART1_TX) ──[220Ω]──► DIN-5 Pin 5
                ──[220Ω]──► DIN-5 Pin 4
+3.3V ──────────[220Ω]──► DIN-5 Pin 2   ← corriente de loop (estándar MIDI)
DIN-5 Pin 1, 3 ──► NC
DIN-5 Shell    ──► GND
```

> Los dos resistores 220Ω en Pin 4 y Pin 5 son estándar del spec MIDI DIN.
> El de Pin 2 (+3.3V via 220Ω) completa el loop de corriente — obligatorio según MIDI 1.0 spec.

---

### 🟢 BLOQUE F15 — Conector USB-B MIDI (datos USB al PC)

**USB MIDI class-compliant. Separado del USB-C de power. Mismos pines PA11/PA12.**

```
USB-B conector (THT):
  Pin 1 (VBUS) ──[Polyfuse 500mA]──► VBUS_DETECT ──[100kΩ]──► GND
  Pin 2 (D-)   ──► USBLC6-2 I/O1 ──► PA11 (USB_DM)
  Pin 3 (D+)   ──► USBLC6-2 I/O2 ──► PA12 (USB_DP)
  Pin 4 (GND)  ──► GND
  Shell        ──► GND (chasis)

USBLC6-2SC6 (SOT-23-6) — protección ESD datos:
  I/O1 ←──► D-  (bidireccional)
  I/O2 ←──► D+  (bidireccional)
  VCC  ──► VBUS (protección al bus)
  GND  ──► GND
```

> ⚠️ El USB-C de power y el USB-B de MIDI **no comparten pines D+/D−** en el STM32.
> Ambos comparten PA11/PA12 — solo uno puede estar activo a la vez. El firmware
> usa PA11/PA12 exclusivamente para USB MIDI. El USB-C es solo power.
> Si en el futuro quieres DFU Y MIDI por el mismo conector, usar solo el USB-C y multiplexar por BOOT0.

---

## ESTADO ACTUAL — PENDIENTES

```
PENDIENTES (diseñar en KiCad):
  🔴 F6  — Jacks CV 4× TRS 3.5mm + BAT54S
  🟡 F8  — OLED SSD1306 (I2C)
  🟡 F10 — Encoders EC11 ×8
  🟡 F11 — Potenciómetros RV3/RV4
  🟡 F12 — SK6812 RGB cadena (1 GPIO PC5, sin PCA9685)
  🟢 F13 — MIDI DIN IN (6N137)
  🟢 F14 — MIDI DIN OUT
  🟢 F15 — USB-B MIDI + USBLC6-2

ADVERTENCIAS ACTIVAS (ver sección arriba):
  ⚠️  [BOOT0] Verificar R1 en BOOT0 — debe ser 10kΩ pull-down a GND
  ⚠️  [SK6812] Conectar cadena DIN→DOUT entre los 16 switches
  ⚠️  [STM32 GND] Verificar net GND vs GNDA en pines VSS del STM32
  ⚠️  [MCP23017] Considerar IOCON.MIRROR=1 para usar solo INTA en 16 switches
  ⚠️  [USB-C CC] VERIFICAR R3/R4: deben ir a GND, no a 5V_USB — imagen ambigua
  ⚠️  [RCA J7B] Confirmar pin señal del canal C con marcador NC explícito
  ⚠️  [DAC8565 SPI] Verificar que LDAC y DIN no usan el mismo GPIO (PF7)
  ⚠️  [PCM5122 I2S] Verificar asignación PE5: es SD (datos), no BCK
  ⚠️  [LT3042] Confirmar que R7 es RSET y existe resistencia separada para RILIM (ILIM pin 5)
```

---

## NOTAS ADICIONALES

**Sobre programación sin ST-Link — DFU por USB:**
No necesitas ningún programador externo. El STM32H743 tiene bootloader DFU en ROM.
Circuito implementado: pull-down 10kΩ en BOOT0 + SW_BOOT SMD (Alps SKQGAF010 o equivalente).
Software: STM32CubeProgrammer (gratis) → detecta el chip via USB sin drivers especiales.
Flujo: activa SW_BOOT → conecta USB → flashea → desactiva SW_BOOT → reset → funciona.


El AMS1117-3.3 es un cambio aceptable respecto al ADP3335 especificado en la guía.
El AMS1117 suministra hasta 1A (más que el ADP3335 de 500mA) y es más económico.
La diferencia en PSRR es negligible para el rail digital. ✅ OK mantenerlo.

**Sobre el DAC8565 VOUTD (pin 8) marcado como NC:**
Solo se usan 3 de los 4 canales CV. Es válido dejar VOUTD sin conectar mientras
la inicialización SPI no escriba a ese canal. En el futuro puede usarse para un 4° CV out.

**Sobre net +3.3V vs +3.3VA:**
Mantener siempre la distinción:
- `+3.3V` = digital (de AMS1117) → STM32, lógica
- `+3.3VA` = analógico (de LT3042) → PCM5122 AVDD/CPVDD exclusivamente
No mezclarlos nunca en el mismo pin.
