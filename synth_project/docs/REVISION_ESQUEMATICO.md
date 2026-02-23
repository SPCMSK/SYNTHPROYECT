# Revisión del Esquemático — DigitalSynth v1.0
**Fecha:** 22 Feb 2026 | **Revisado contra:** `05_GUIA_ESQUEMATICO.md` v2.0
**Estado:** ✅ Errores críticos y menores CORREGIDOS — pendiente completar bloques faltantes

---

## RESUMEN EJECUTIVO

| Bloque | Estado | Notas |
|---|---|---|
| USB-C (power) | ✅ CORREGIDO | CC1/CC2 → GND ✓, polyfuse único 2A ✓ |
| STM32H743 | ✅ CORREGIDO | BOOT0 con SW_BOOT SMD + pull-down ✓, R2 eliminado ✓ |
| Decoupling STM32 | ✅ CORREGIDO | VCAP1/VCAP2 añadidos ✓, 22Ω cristal ✓ |
| LDO 3.3VA (LT3042) | ✅ CORRECTO | RILIM 360kΩ añadido ✓ |
| LDO 5V→3.3V (AMS1117) | ✅ ACEPTABLE | Bypass cerámicos 100nF añadidos ✓ |
| PCM5122 (DAC audio) | ✅ CORREGIDO | XSMT → +3.3VA ✓ |
| DAC8565 (CV) | ✅ CORREGIDO | Bloque +5VA añadido ✓, nets renombradas a CV1-4 ✓ |

**Bloques corregidos: 7 / 7 ✅ — Bloques faltantes por diseñar: 11 / ~18**

---

## ERRORES CRÍTICOS — ✅ TODOS CORREGIDOS

---

### ✅ ERROR 1 — USB-C: CC1 y CC2 conectados a VBUS (CORREGIDO)

**Imagen:** `USBC.png`

**Lo que tiene:**
```
5V_USB → R3 (5K1) → CC2 (B5)
5V_USB → R4 (5K1) → CC1 (A5)
```

**Lo que debe ser:**
```
CC1 (A5) → R4 (5K1) → GND
CC2 (B5) → R3 (5K1) → GND
```

**Por qué es crítico:**
Los resistores CC son **pull-DOWN a GND** (Rd = 5.1kΩ). Esto le dice al cargador USB-C:
*"soy un dispositivo, dame 5V/1.5A"*.
Con pull-UP a VBUS, el cargador verá los CC como señal de HOST/DFP — puede no entregar
corriente, rechazar la conexión, o en el peor caso dañar el cargador.

**Fix aplicado:** R3 y R4 redirigidos a GND. ✅

---

### ✅ ERROR 2 — USB-C: Dos polyfuses en paralelo (CORREGIDO)

**Imagen:** `USBC.png`

**Lo que tiene:** F1 (2A) y F2 (2A) en paralelo → dos paths de VBUS a 5V_BUS.

**Problema:** En paralelo suman → fusible efectivo de 4A. El diseño solo consume ~400mA.
Un fallo de cortocircuito real no abrirá el fusible hasta 4A — demasiado tarde para proteger
el circuito.

**Fix aplicado:** F2 eliminado. Queda un único polyfuse 2A. ✅

---

### ✅ ERROR 3 — STM32: BOOT0 con switch SMD (CORREGIDO)

**Imagen:** `STM32.png`

**Buenas noticias:** No necesitas ST-Link ni ningún programador externo.
El STM32H743 tiene un **bootloader DFU en ROM** que activa el USB-C que ya tienes.
Flasheas con **STM32CubeProgrammer** (gratis, de ST) directamente por USB.

**Solución implementada:** Switch SMD (SW_BOOT) en lugar de jumper.

**Circuito final:**
```
BOOT0 (pin 94) ──[10kΩ R_BOOT, 0402]──► GND    ← pull-down normal = arranca firmware
BOOT0 (pin 94) ──────────────────────► SW_BOOT (SPST, SMD) ──► +3.3V
```

**Switch recomendado:** Alps SKQGAF010 (SMD, 4 pines, 100mΩ, footprint 3.9×2.9mm)
or cualquier SPST SMD de 3.3V/200mA. Uno NO-momentáneo es más cómodo (no hay que mantenerlo pulsado).

**Cómo programar sin ST-Link:**
1. Activa SW_BOOT (BOOT0 = HIGH)
2. Conecta USB-C al PC
3. Abre **STM32CubeProgrammer** → selecciona USB → Connect
4. Carga tu `.hex` o `.elf` → Download
5. Desactiva SW_BOOT (BOOT0 = LOW)
6. Pulsa Reset → el STM32 arranca con tu firmware

**Fix aplicado:** R1 pull-up eliminado. R_BOOT 10kΩ pull-down + SW_BOOT SMD añadido. ✅

---

### ✅ ERROR 4 — STM32: Resistor externo 1K5 en USB_DP (CORREGIDO)

**Imagen:** `STM32.png`

**Lo que tiene:** R2 (1K5) entre PA12/USB_D+ y +3.3V.

**Problema:** El STM32H743 tiene **pull-up interno de 1.5kΩ en D+** que activa
automáticamente tanto en modo DFU (bootloader ROM) como en modo USB MIDI normal.
Colocar además un 1.5kΩ externo resulta en **750Ω efectivos** (paralelo),
violando la spec USB FS (debe ser 1.5kΩ ±5%). El PC verá el dispositivo como
defectuoso o no enumerará — esto afecta tanto DFU como MIDI.

**Fix aplicado:** R2 eliminado del esquemático. ✅

---

### ✅ ERROR 5 — PCM5122: Pin XSMT (CORREGIDO)

**Imagen:** `DAC AUDIO.png`

**Lo que tiene en el esquemático actual:** XSMT (pin 25) → `PC3_GPIO` ✓

**Esto es correcto y mejor que +3.3VA fijo** — permite control de mute por firmware.

⚠️ **TRAMPA CRÍTICA DE FIRMWARE:** Si PC3 arranca en LOW (estado por defecto de los GPIO
del STM32 al encender), el DAC estará **muteado hasta que el firmware lo desactive**.
Esto puede confundirse con un fallo de audio al hacer pruebas iniciales.

**En CubeMX/código de inicialización, OBLIGATORIO:**
```c
// Inmediatamente después de init, antes de reproducir audio:
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);  // XSMT = HIGH = unmute
```
O configurar PC3 como output con **valor inicial HIGH** en CubeMX (Output level: High).

**Fix aplicado:** XSMT conectado a PC3_GPIO ✅ — añadir inicialización HIGH en firmware.

---

### ✅ ERROR 6 — Fuente +5VA (CORREGIDO)

**Imagen:** `DAC.png` (DAC8565, pin AVDD = "+5VA")

**Problema:** El DAC8565 usa la net `+5VA` en su AVDD, pero no existe ningún bloque en el
esquemático que genere `+5VA`. El LT3042 genera `+3.3VA`, y el AMS1117 genera `+3.3V`.

**Lo que debe existir (bloque faltante):**
```
5V_BUS → [Ferrite 600Ω@100MHz] → +5VA (para DAC8565 AVDD únicamente)
                                     + C (100nF + 4.7µF a GNDA)
```
La fuente `+5VA` es simplemente 5V_BUS filtrado con ferrite — **no necesita LDO**.
El DAC8565 en salida CV tiene ruido no crítico, el ferrite es suficiente.

**Fix aplicado:** Bloque ferrite + caps +5VA añadido. Nets CV1_OUT/CV2_OUT/CV3_OUT/CV4_OUT renombradas. ✅

---

## ERRORES MENORES — ✅ TODOS CORREGIDOS

---

### ✅ MENOR 1 — Decoupling STM32: VCAP1 y VCAP2 (CORREGIDO)

**Imagen:** `DECOUPLING STM.png`

Los pines VCAP1 (pin 48) y VCAP2 (pin 73) del STM32H743 **no aparecen en el bloque de
decoupling**. Estos pines son para el regulador interno del core a 1.2V y requieren:

```
VCAP1 (pin 48) → C_VCAP1 (2.2µF ceramic, X5R) → GND
VCAP2 (pin 73) → C_VCAP2 (2.2µF ceramic, X5R) → GND
```

**Fix aplicado:** C_VCAP1 y C_VCAP2 (2.2µF X5R) añadidos en bloque decoupling. ✅

---

### ✅ MENOR 2 — Cristal: Resistor serie 22Ω (CORREGIDO)

**Imagen:** `STM32.png` (cristal Y1 25MHz)

La guía especifica un resistor de 22Ω en serie en OSC_IN (HSE_IN, pin 12). Este resistor
limita la corriente que fluye por el cristal y previene sobreoscilación en cristales de baja
frecuencia relativa. Sin él el cristal puede oscilar en armónicos o tener problemas de
arranque.

```
PA oscilador → [22Ω] → OSC_IN del cristal
```

**Fix aplicado:** Resistor 22Ω serie en OSC_IN añadido. ✅

---

### ✅ MENOR 3 — Decoupling STM32: Condensadores VDD (CORREGIDO)

**Imagen:** `DECOUPLING STM.png`

El STM32H743VIT tiene **11 pines VDD separados**. El bloque actual tiene 7 condensadores
(1× 10µF + 6× 100nF). Recomendado por ST: un condensador 100nF por pin VDD físico.

Distribución sugerida:
- **VDD (×11):** 11× 100nF + 2× 10µF (bulk)
- **VDDA (×1):** 1× 1µF + 1× 10nF (ya tienes esto con FB1 ✓)

**Fix aplicado:** Condensadores 100nF distribuidos por los 11 pines VDD. ✅

---

### ✅ MENOR 4 — LT3042: Pin ILIM (CORREGIDO)

**Imagen:** `LDO 3.3 VA.png`

El pin ILIM (pin 5) parece no estar conectado (círculo abierto). En el LT3042, ILIM flotante
tiene comportamiento indefinido según el datasheet.

**Opciones:**
- `ILIM → [360kΩ] → GND` → límite de ~200mA ← **recomendado**
- `ILIM → IN` → deshabilita límite de corriente (máximo hardware)

**Fix aplicado:** RILIM = 360kΩ a GND añadido. ✅

---

### ✅ MENOR 5 — Naming GND analógico (CORREGIDO)

A lo largo del esquemático se usan **tres nombres distintos para el mismo plano analógico:**
- `AGND` (en bloque PCM5122)
- `GNDA` (en bloques DAC8565 y filtros de salida)
- `GND` (en LT3042, AMS1117, y algún bloque digital)

En KiCad estos son **nets distintos** si no están conectados. Si AGND ≠ GNDA, el PCM5122
y el filtro de salida no comparten plano de tierra → distorsión y ruido.

**Fix aplicado:** Todas las nets analógicas unificadas a `GNDA`. Verificar con ERC de KiCad que no queden nets `AGND` huérfanas. ✅

---

### ✅ MENOR 6 — DAC8565: Naming de salidas (CORREGIDO)

**Imagen:** `DAC.png`

Las salidas del DAC8565 se llaman `VCF` y `VCA`, pero en la arquitectura digital v2.0 **no
existe cadena analógica VCF/VCA**. Los nombres correctos son salidas CV externas:

| Actual | Correcto | Destino |
|---|---|---|
| VCF | CV1_OUT | Jack CV 1 (Pitch voz 1) |
| VCA | CV2_OUT | Jack CV 2 (Pitch voz 2) |
| PITCH/CV | CV3_OUT | Jack CV 3 (Mod/Env) |
| (VOUTD/NC) | CV4_OUT | Jack CV 4 (libre) |

**Fix aplicado:** Nets renombradas a CV1_OUT–CV4_OUT. ✅

---

### ✅ MENOR 7 — PCM5122: Condensadores CAPP/CAPM (CORREGIDO)

**Imagen:** `DAC AUDIO.png`

C32 (2.2µF) entre CAPP (pin 2) y CAPM (pin 4) — el `+` del electrolítico debe estar en
CAPP. Verificar la polaridad en el symbol de KiCad. Si está invertida, el condensador se
polariza al revés durante operación.

**Fix aplicado:** Polaridad de C32 verificada y corregida (+ en CAPP). ✅

---

### ✅ MENOR 8 — AMS1117: Bypass cerámicos (CORREGIDO)

**Imagen:** `LDO 5 TO 3.3.png`

AMS1117 solo tiene C17/C18 (22µF, probablemente electrolíticos). El AMS1117 requiere
también condensadores cerámicos de bypass para estabilidad:

```
Entrada: 100nF cerámico en paralelo con C17
Salida:  100nF cerámico en paralelo con C18
```

**Fix aplicado:** 100nF cerámico añadido en entrada y salida del AMS1117. ✅

---

## LO QUE ESTÁ CORRECTO ✅

| Bloque | Qué está bien |
|---|---|
| **LT3042** | RSET = 33.2kΩ → Vout = 3.3V exacto ✓, C_SET (0.47µF) ✓, ferrite en entrada ✓ |
| **PCM5122 alimentación** | AVDD y CPVDD → +3.3VA (correcto para v2.0, no +5V) ✓ |
| **PCM5122 I2S** | SCK/BCK/LRCK/DIN conectados con nets correctas ✓ |
| **PCM5122 salida** | 470Ω + 2.2nF + 100Ω bifurcación a PLUG y RCA ✓ |
| **PCM5122 modo** | MODE1/MODE2 a +3.3VA = I2S slave ✓ |
| **DAC8565 SPI** | PA5/PA7/PB6/PB7 correctos para SCLK/DIN/SYNC/LDAC ✓ |
| **DAC8565 IOVDD** | +3.3V (digital) separado de AVDD (+5VA) ✓ |
| **DAC8565 ENABLE/RSTSEL** | Ambos a GND (activo LOW, siempre habilitado) ✓ |
| **DAC8565 Vref** | VREFH/VREFOUT con 100nF + 10µF ✓ (referencia interna 2.5V) |
| **DAC8565 RST** | Pull-up 10kΩ a +3.3V + GPIO PC4 para reset controlado ✓ |
| **Cristal** | 25MHz, 14pF carga, conectado a HSE_IN/HSE_OUT ✓ |
| **USB-C datos** | D+/D- con nets USB_D+ y USB_D- ✓ |
| **USB-C GND** | Pin GND a tierra ✓ |
| **Decoupling VDDA** | Ferrite + 10nF + 1µF + 1µF para VDDA del STM32 ✓ |
| **AMS1117** | Diagrama correcto, rutas de alimentación bien trazadas ✓ |

---

## BLOQUES FALTANTES — CONEXIONES DETALLADAS

> Úsalos como guía directa en KiCad. Cada bloque está listo para copiar pin a pin.

---

### 🔴 BLOQUE F1 — VCAP1 y VCAP2 (agregar al bloque DECOUPLING STM)

**Obligatorio. Sin esto el STM32H743 puede no arrancar.**

```
Pin 48 (VCAP1) ──► C_VCAP1 [2.2µF, cerámico X5R, 0805] ──► GND
Pin 73 (VCAP2) ──► C_VCAP2 [2.2µF, cerámico X5R, 0805] ──► GND
```

> ⚠️ NO conectar a VDD. Estos pines son la salida del regulador interno de 1.2V del core.
> Condensadores deben ser cerámicos (no electrolíticos). 2.2µF exacto, no más de 4.7µF.

---

### 🔴 BLOQUE F2 — Fuente +5VA (ferrite simple desde 5V_BUS)

**Requerido por DAC8565 AVDD. Sin este bloque la net +5VA queda sin fuente.**

```
5V_BUS ──[FB4, Ferrite 600Ω@100MHz, 500mA]──► +5VA
                                               ├── C_5VA_1 [4.7µF, X5R, 0805] ──► GNDA
                                               └── C_5VA_2 [100nF, C0G, 0402] ──► GNDA
```

> No necesita LDO. El DAC8565 genera CV para gear externo — ruido no crítico.
> Ferrite recomendado: Murata BLM21PG601SN1D o equivalente 0805.

---

### 🔴 BLOQUE F3 — BOOT0 + Switch SMD DFU (programación por USB sin ST-Link)

**Solución implementada con switch SMD en lugar de jumper.**

```
BOOT0 (pin 94 STM32) ──[R_BOOT, 10kΩ, 0402]──► GND
BOOT0 (pin 94 STM32) ──────────────────────► SW_BOOT [Switch SPST SMD] ──► +3.3V
```

**Switch recomendado:** Alps SKQGAF010 (SMD SPST, 3.9×2.9mm, panel ON/OFF)
→ Usar un switch NO-momentáneo (toggle/sliding) para no tener que mantenerlo pulsado durante la carga.

**Cómo usar SW_BOOT:**
- Switch OFF (normal): BOOT0 = LOW → arranca firmware desde Flash
- Switch ON: BOOT0 = HIGH → arranca DFU → flashear con STM32CubeProgrammer por USB

---

### 🔴 BLOQUE F4 — USBLC6-2 ESD (protección datos USB)

**Va entre el conector USB-C y los pines PA11/PA12 del STM32.**

```
                    USBLC6-2SC6 (SOT-23-6)
USB_D-  ──────────► I/O1 ──────────────────► PA11 (USB_DM)
USB_D+  ──────────► I/O2 ──────────────────► PA12 (USB_DP)
5V_BUS  ──────────► VCC  (protección VBUS)
GND     ──────────► GND
```

> Datasheet: STMicroelectronics USBLC6-2SC6
> Footprint: SOT-23-6. Orientación crítica — verificar I/O1→D- e I/O2→D+ en el symbol.

---

### ✅ BLOQUE F5 — Jacks Salida Audio (COMPLETADO)

**Las nets L_CHAN y R_CHAN ya salen del filtro RC del PCM5122 y se bifurcan a PLUG y RCA.**

**¿Se necesita op-amp?**
**No.** El PCM5122 en modo single-ended (que es lo que tienes) tiene buffer de salida
interno. Puede manejar directamente las entradas de cualquier equipo (mezcladores, interfaces,
monitores activos) que típicamente son 10kΩ–100kΩ. Sin op-amp necesario. ✓

**Circuito actual — ya correcto ✓**
```
                R8 470Ω          C36 2n2        R10 100Ω
PCM5122 OUTR ──[R8, 470Ω]──┬──[C36, 2n2]──► GNDA     ├──────────► R_CHAN_PLUG ──► J_TRS_R Tip
                            │                          R11 100Ω
                            └──────────────────────────┴──────────► R_CHAN_RCA  ──► J_RCA_R Tip

                R9 470Ω          C40 2n2        R12 100Ω
PCM5122 OUTL ──[R9, 470Ω]──┬──[C40, 2n2]──► GNDA     ├──────────► L_CHAN_PLUG ──► J_TRS_L Tip
                            │                          R13 100Ω
                            └──────────────────────────┴──────────► L_CHAN_RCA  ──► J_RCA_L Tip

Todos los Sleeve/Shell ──► GNDA
```

**El filtro 470Ω + 2.2nF** es un low-pass con fc = 1/(2π × 470 × 2.2e-9) ≈ **154 kHz** — limpia
aliasing de ultrasonido sin tocar el audio audible (20Hz–20kHz). ✓

**Los 100Ω en serie** hacia cada jack aislan capacitancias de cable y protegen contra cortocircuito. ✓

**Símbolo KiCad usado:** `Connector_Audio:NRJ4HF-1` (Neutrik, horizontal, mono) × 2
Footprint incluido: `Connector_Audio:Jack_6.35mm_Neutrik_NRJ4HF-1_Horizontal` ✓

```
J8 — Canal DERECHO               J6 — Canal IZQUIERDO
──────────────────               ────────────────────
T  ──► R_CHAN_PLUG                T  ──► L_CHAN_PLUG
S  ──► GNDA                       S  ──► GNDA
G  ──► GNDA  (chassis)            G  ──► GNDA  (chassis)
TN ──► NC                         TN ──► NC
```

> TN es el contacto normalmente cerrado que se abre al insertar el jack — no conectar.
> G es el blindaje metálico del conector — conectar a GNDA igual que S.
> ⚠️ No añadir BAT54S ni diodos en señal de audio — distorsionan.
> Footprint RCA recomendado: `Connector_Audio:Jack_RCA_CUI_RCJ-014` — PCB mount.

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

### 🟡 BLOQUE F7 — W25Q128 Flash SPI (16MB)

**Almacena patches, wavetables y datos de secuenciador. SPI2 del STM32.**

```
STM32H743 SPI2          W25Q128JVSIQ (SOIC8)
──────────────           ────────────────────
PB13 (SPI2_SCK)  ──────► Pin 6 (CLK)
PB5  (SPI2_MOSI) ──────► Pin 5 (DI)
PB4  (SPI2_MISO) ◄────── Pin 2 (DO)
PB14 (GPIO OUT)  ──────► Pin 1 (~CS)

Pin 1 (~CS)   ── [100kΩ pull-up] ──► +3.3V  (evita acceso accidental al arrancar)
Pin 3 (~WP)   ──────────────────► +3.3V      (write-protect deshabilitado = siempre escribible)
Pin 7 (~HOLD) ──[10kΩ pull-up]──► +3.3V     (no usar hold)
Pin 8 (VCC)   ──► +3.3V + C [100nF, C0G] ──► GND
Pin 4 (GND)   ──► GND
```

> LCSC: C97521 (W25Q128JVSIQ, SOIC8). Precio: ~$1.50
> Velocidad máx SPI2: 50MHz. En CubeMX: SPI2 → Full-Duplex Master → Prescaler /4 = 120MHz/4 = 30MHz.

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

### 🟡 BLOQUE F9 — MCP23017 Expansor GPIO (switches y botones)

**16 GPIO extra por I2C. Escanea 16 step-switches + botones de función.**

```
STM32H743 I2C1           MCP23017 (SSOP28, dirección I2C: 0x20)
──────────────            ────────────────────────────────────
PB8 (SCL) ──────────────► Pin 12 (SCL)
PB9 (SDA) ──────────────► Pin 13 (SDA)
PC0 (GPIO IN, EXTI) ◄──── Pin 20 (INT_A)    ← interrupción, activo LOW

Pin 9  (VDD)      ──► +3.3V + C [100nF] ──► GND
Pin 10 (VSS)      ──► GND
Pin 18 (RESET_n)  ──[10kΩ pull-up]──► +3.3V   (sin reset externo)
Pin 15 (A0)       ──► GND  ┐
Pin 16 (A1)       ──► GND  │→ dirección = 0x20
Pin 17 (A2)       ──► GND  ┘

GPA0–GPA7 (Pins 21–28): 8× step switches, steps 1–8
GPB0–GPB7 (Pins 1–8):   8× step switches, steps 9–16
  Cada switch: Pin GPxy ──► SW_n ──► GND   (pull-up interno del MCP23017 habilitado por I2C)
```

> Los pull-ups de los switches se habilitan por software (registro GPPU = 0xFF).
> Sin resistores pull-up externos en el PCB para los switches. ✓ Ahorra espacio.

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

### 🟢 BLOQUE F12 — PCA9685 Driver LEDs PWM (16 canales)

**LEDs del secuenciador. Mismo bus I2C1.**

```
STM32H743 I2C1          PCA9685 (TSSOP28, dirección: 0x40)
──────────────           ──────────────────────────────────
PB8 (SCL) ─────────────► Pin 19 (SCL)
PB9 (SDA) ─────────────► Pin 18 (SDA)
PD12 (GPIO OUT) ────────► Pin 20 (~OE)  + [10kΩ pull-down a GND] ← LEDs ON por defecto

Pin 1–6  (A0–A5) ──► GND  → dirección 0x40
Pin 17 (VDD)  ──► +3.3V + C [100nF] ──► GND
Pin 16 (GND)  ──► GND
Pin 21 (EXTCLK) ──► GND (oscilador interno)

── Salidas LED (misma conexión ×16) ──
Pin LED_n ──[R 330Ω, 0402]──► LED_ANODE ──► LED ──► GND
  Cálculo: (3.3V - 2.0V_fwd) / 330Ω = 3.9mA (visible, bajo consumo)
  Para más brillo: usar 150Ω → 8.7mA
```

> Si quieres LEDs bicolor (rojo + verde por paso): necesitas 2× PCA9685.
> Segundo en dirección 0x41 (A0 → +3.3V, A1–A5 → GND).

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

## PLAN DE CORRECCIONES — ESTADO ACTUAL

```
CORRECCIONES APLICADAS ✅:
  1. ✅ CC1/CC2 en USB-C → GND (pull-down)
  2. ✅ BOOT0: pull-down 10kΩ + SW_BOOT SMD (en lugar de jumper)
  3. ✅ XSMT del PCM5122 → +3.3VA
  4. ✅ Bloque +5VA: ferrite BLM21 desde 5V_BUS + 4.7µF + 100nF a GNDA
  5. ✅ R2 (1K5) en USB_DP eliminado
  6. ✅ F2 (polyfuse duplicado) eliminado — queda 1× polyfuse 2A
  7. ✅ VCAP1 y VCAP2 (2.2µF X5R) añadidos
  8. ✅ Resistor serie 22Ω en OSC_IN del cristal
  9. ✅ Naming AGND → GNDA unificado
  10. ✅ RILIM 360kΩ a GND en LT3042

BLOQUES PENDIENTES (diseñar en KiCad):
  11. ○ F1 — VCAP conexiones en bloque decoupling
  12. ○ F4 — USBLC6-2 ESD en USB datos
  13. ✅ F5 — Jacks audio: 2× NRJ4HF-1 (L/R) + RCJ-32265 triple RCA (C sin conectar)
  14. ○ F6 — Jacks CV 4× TRS 3.5mm + BAT54S
  15. ○ F7 — Flash W25Q128 (SPI2)
  16. ○ F8 — OLED SSD1306 (I2C)
  17. ○ F9 — MCP23017 switches (I2C)
  18. ○ F10 — Encoders EC11 ×8
  19. ○ F11 — Potenciómetros RV3/RV4
  20. ○ F12 — PCA9685 + LEDs ×16
  21. ○ F13 — MIDI DIN IN (6N137)
  22. ○ F14 — MIDI DIN OUT
  23. ○ F15 — USB-B MIDI + USBLC6-2
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
