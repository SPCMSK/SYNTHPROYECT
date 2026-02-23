# Revisión del Esquemático — DigitalSynth v1.0
**Fecha:** 22 Feb 2026 | **Revisado contra:** `05_GUIA_ESQUEMATICO.md` v2.0

---

## RESUMEN EJECUTIVO

| Bloque | Estado | Errores críticos |
|---|---|---|
| USB-C (power) | ⚠️ ERRORES | CC1/CC2 invertidos — no cargará correctamente |
| STM32H743 | ⚠️ ERRORES | BOOT0 pull-up incorrecto, D+ pull-up externo innecesario |
| Decoupling STM32 | ⚠️ INCOMPLETO | Faltan VCAP1/VCAP2, resistor serie cristal |
| LDO 3.3VA (LT3042) | ✅ CORRECTO | Solo detalle menor en ILIM |
| LDO 5V→3.3V (AMS1117) | ✅ ACEPTABLE | Cambio de ADP3335 → AMS1117 (OK) |
| PCM5122 (DAC audio) | ⚠️ ERROR | XSMT flotante — DAC estará en mute permanente |
| DAC8565 (CV) | ⚠️ OBSERVACIONES | Falta fuente +5VA, naming VCF/VCA confuso |

**Bloques diseñados: 7 / ~18 totales**

---

## ERRORES CRÍTICOS (deben corregirse antes de fabricar)

---

### ❌ ERROR 1 — USB-C: CC1 y CC2 conectados a VBUS (FATAL)

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

**Fix en KiCad:** Girar/rerouter R3 y R4 para que el otro extremo vaya a GND, no a 5V_USB.

---

### ❌ ERROR 2 — USB-C: Dos polyfuses en paralelo (F1 + F2)

**Imagen:** `USBC.png`

**Lo que tiene:** F1 (2A) y F2 (2A) en paralelo → dos paths de VBUS a 5V_BUS.

**Problema:** En paralelo suman → fusible efectivo de 4A. El diseño solo consume ~400mA.
Un fallo de cortocircuito real no abrirá el fusible hasta 4A — demasiado tarde para proteger
el circuito.

**Fix:** Eliminar uno de los dos polyfuses. Usar solo **1× polyfuse 2A** (RXEF020 o similar).

---

### ❌ ERROR 3 — STM32: BOOT0 puede estar en estado incorrecto

**Imagen:** `STM32.png`

**Lo que se ve:** R1 en pin 94 (BOOT0) con flecha hacia +3.3V.

Si R1 es un pull-UP a +3.3V:
- BOOT0 = HIGH al encender → **STM32 arranca en modo bootloader DFU**, no en flash.
- El firmware compilado nunca ejecuta.

**Lo que debe ser:**
```
BOOT0 (pin 94) → R1 (10kΩ) → GND   ← pull-DOWN = boot desde Flash
```
El 10kΩ a GND es suficiente. Si en el futuro necesitas entrar a DFU, se conecta
temporalmente a 3.3V (o via jumper de 2 pines).

**Fix:** Confirmar en KiCad que R1 va a GND, no a +3.3V.

---

### ❌ ERROR 4 — STM32: Resistor externo 1K5 en USB_DP (PA12)

**Imagen:** `STM32.png`

**Lo que tiene:** R2 (1K5) entre PA12/USB_D+ y +3.3V.

**Problema:** El STM32H743 OTG_FS tiene **pull-up interno de 1.5kΩ en D+** habilitado
por software. Colocar además un 1.5kΩ externo resulta en 750Ω efectivos (paralelo), lo que
viola la especificación USB FS (debe ser 1.5kΩ ±5%). El host verá el dispositivo
como defectuoso o no enumerará.

**Fix:** **Eliminar R2 completamente.** El pull-up lo gestiona CubeMX/HAL automáticamente.

---

### ❌ ERROR 5 — PCM5122: Pin XSMT flotante — salida en mute permanente

**Imagen:** `DAC AUDIO.png`

**Lo que tiene:** Pin 25 (XSMT) con una flecha pero sin net conectada clara.

**Problema:** XSMT = 0 (LOW o flotante) → **DAC silenciado permanentemente**.
El chip funciona, recibe I2S, pero bloquea la salida analógica internamente.

**Lo que debe ser:**
```
XSMT (pin 25) → [10kΩ pull-up] → +3.3VA
               también → PC3_GPIO   ← para soft-mute desde firmware
```
Si no se necesita control de mute desde GPIO, simplemente conectar directamente a +3.3VA.

**Fix:** Conectar XSMT a +3.3VA (con o sin pull-up + GPIO según se prefiera).

---

### ❌ ERROR 6 — Fuente +5VA no definida en el esquemático

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

---

## ERRORES MENORES (corregir antes del layout)

---

### ⚠️ MENOR 1 — Decoupling STM32: Faltan VCAP1 y VCAP2

**Imagen:** `DECOUPLING STM.png`

Los pines VCAP1 (pin 48) y VCAP2 (pin 73) del STM32H743 **no aparecen en el bloque de
decoupling**. Estos pines son para el regulador interno del core a 1.2V y requieren:

```
VCAP1 (pin 48) → C_VCAP1 (2.2µF ceramic, X5R) → GND
VCAP2 (pin 73) → C_VCAP2 (2.2µF ceramic, X5R) → GND
```

**⛔ CRÍTICO:** Sin estos condensadores el STM32H743 puede no arrancar o tener
comportamiento inestable. Están documentados como obligatorios en el datasheet de STM.

---

### ⚠️ MENOR 2 — Cristal: Falta resistor serie en OSC_IN

**Imagen:** `STM32.png` (cristal Y1 25MHz)

La guía especifica un resistor de 22Ω en serie en OSC_IN (HSE_IN, pin 12). Este resistor
limita la corriente que fluye por el cristal y previene sobreoscilación en cristales de baja
frecuencia relativa. Sin él el cristal puede oscilar en armónicos o tener problemas de
arranque.

```
PA oscilador → [22Ω] → OSC_IN del cristal
```

---

### ⚠️ MENOR 3 — Decoupling STM32: C_DEC_VDD insuficiente

**Imagen:** `DECOUPLING STM.png`

El STM32H743VIT tiene **11 pines VDD separados**. El bloque actual tiene 7 condensadores
(1× 10µF + 6× 100nF). Recomendado por ST: un condensador 100nF por pin VDD físico.

Distribución sugerida:
- **VDD (×11):** 11× 100nF + 2× 10µF (bulk)
- **VDDA (×1):** 1× 1µF + 1× 10nF (ya tienes esto con FB1 ✓)

---

### ⚠️ MENOR 4 — LT3042: Pin ILIM flotante

**Imagen:** `LDO 3.3 VA.png`

El pin ILIM (pin 5) parece no estar conectado (círculo abierto). En el LT3042, ILIM flotante
tiene comportamiento indefinido según el datasheet.

**Opciones:**
- `ILIM → [360kΩ] → GND` → límite de ~200mA ← **recomendado**
- `ILIM → IN` → deshabilita límite de corriente (máximo hardware)

---

### ⚠️ MENOR 5 — Naming inconsistente de GND analógico

A lo largo del esquemático se usan **tres nombres distintos para el mismo plano analógico:**
- `AGND` (en bloque PCM5122)
- `GNDA` (en bloques DAC8565 y filtros de salida)
- `GND` (en LT3042, AMS1117, y algún bloque digital)

En KiCad estos son **nets distintos** si no están conectados. Si AGND ≠ GNDA, el PCM5122
y el filtro de salida no comparten plano de tierra → distorsión y ruido.

**Fix:** Unificar todo a `GNDA` para el analógico y `GND` para el digital. Verificar con
el ERC que no haya nets aisladas.

---

### ⚠️ MENOR 6 — DAC8565: Naming de salidas VCF/VCA confuso

**Imagen:** `DAC.png`

Las salidas del DAC8565 se llaman `VCF` y `VCA`, pero en la arquitectura digital v2.0 **no
existe cadena analógica VCF/VCA**. Los nombres correctos son salidas CV externas:

| Actual | Correcto | Destino |
|---|---|---|
| VCF | CV1_OUT | Jack CV 1 (Pitch voz 1) |
| VCA | CV2_OUT | Jack CV 2 (Pitch voz 2) |
| PITCH/CV | CV3_OUT | Jack CV 3 (Mod/Env) |
| (VOUTD/NC) | CV4_OUT | Jack CV 4 (libre) |

---

### ⚠️ MENOR 7 — PCM5122: Condensadores CAPP/CAPM

**Imagen:** `DAC AUDIO.png`

C32 (2.2µF) entre CAPP (pin 2) y CAPM (pin 4) — el `+` del electrolítico debe estar en
CAPP. Verificar la polaridad en el symbol de KiCad. Si está invertida, el condensador se
polariza al revés durante operación.

---

### ⚠️ MENOR 8 — AMS1117: Faltan bypass cerámicos

**Imagen:** `LDO 5 TO 3.3.png`

AMS1117 solo tiene C17/C18 (22µF, probablemente electrolíticos). El AMS1117 requiere
también condensadores cerámicos de bypass para estabilidad:

```
Entrada: 100nF cerámico en paralelo con C17
Salida:  100nF cerámico en paralelo con C18
```

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

## BLOQUES FALTANTES POR DISEÑAR

Los siguientes bloques **no están en ninguna imagen** — deben crearse:

### Alta prioridad (sin estos el proyecto no funciona)
- [ ] **SWD connector** (10-pin ARM, Samtec FTSH-105) — para programar el STM32
- [ ] **VCAP capacitors** (2× 2.2µF a GND, pines 48 y 73) — puede ir en bloque decoupling
- [ ] **Fuente +5VA** (ferrite desde 5V_BUS + caps) — requerida por DAC8565
- [ ] **USBLC6-2 ESD** en USB_D+/USB_D− (entre conector USB y STM32) — protección ESD
- [ ] **Jacks de salida audio** (TRS 6.35mm + RCA) con las nets L/R_CHAN_PLUG y L/R_CHAN_RCA
- [ ] **Jacks CV out** (4× TRS 3.5mm) con BAT54S anti-reversa en cada uno

### Media prioridad (funcionalidad del sintetizador)
- [ ] **W25Q128 Flash SPI** (SPI2: PB13/PB5/PB4/PB14) — 16MB almacenamiento de patches
- [ ] **SSD1306 OLED** (I2C: PB8/PB9, 0x3C) — display 128×64
- [ ] **MCP23017** (I2C: PB8/PB9, 0x20) — 16 step switches + botones
- [ ] **Encoders EC11 ×8** con pull-ups 10kΩ y caps 100nF anti-rebote
- [ ] **Potenciómetros** RV3 (Master Volume → PA0) y RV4 (Filter Cutoff → PA1)

### Baja prioridad (UI completa)
- [ ] **PCA9685** (I2C: PB8/PB9, 0x40) — driver 16× LED con PWM
- [ ] **LEDs ×16** con resistores 330Ω (o bicolor rojo/verde ×16)
- [ ] **MIDI DIN IN** — optoacoplador 6N137, diodo 1N4148, USART1_RX (PA10)
- [ ] **MIDI DIN OUT** — 2× 220Ω, USART1_TX (PA9)
- [ ] **Conector USB-B MIDI** (separado del USB-C de power) + USBLC6-2

---

## PLAN DE CORRECCIONES — ORDEN SUGERIDO

```
ANTES DE CONTINUAR DISEÑANDO:
  1. Corregir CC1/CC2 en USB-C (pull-down a GND, no pull-up) — CRÍTICO
  2. Corregir BOOT0 R1 (pull-down a GND) — CRÍTICO
  3. Conectar XSMT del PCM5122 a +3.3VA o GPIO — CRÍTICO
  4. Añadir bloque +5VA (ferrite desde 5V_BUS) — CRÍTICO
  5. Eliminar R2 (1K5) en USB_DP del STM32
  6. Eliminar uno de los dos polyfuses USB-C
  7. Añadir VCAP1 y VCAP2 (2.2µF)
  8. Añadir resistor serie 22Ω en OSC_IN del cristal
  9. Unificar naming AGND → GNDA en bloque PCM5122
  10. Añadir RILIM (360kΩ a GND) en LT3042

DESPUÉS (bloques nuevos):
  11. SWD connector
  12. USBLC6-2 en USB datos
  13. Jacks audio + CV con protección
  14. Flash W25Q128
  15. OLED + MCP23017
  16. Encoders + potenciómetros
  17. PCA9685 + LEDs
  18. MIDI DIN IN/OUT + USB-B MIDI
```

---

## NOTAS ADICIONALES

**Sobre el AMS1117 vs ADP3335:**
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
