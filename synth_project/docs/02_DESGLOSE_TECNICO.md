# Desglose Técnico Completo
## HybridSynth v1.0 — Ingeniería, Riesgos y Complicaciones

---

## 1. Arquitectura del Sistema

### Diagrama de Bloques Completo

```
                        ┌─────────────────────────────────────────────┐
                        │              STM32H743 @ 480MHz              │
                        │                                             │
  USB Type B ──────────►│ USB OTG FS ──► USB MIDI Parser             │
  MIDI TRS IN ──────────►│ UART1 (optoacoplado) ──► MIDI Parser       │
  CV/Gate IN ───────────►│ ADC3 (0–5V) ──► CV Input Handler          │
                        │                          │                  │
  8× Encoders EC11 ──────►│ GPIO + TIM (quadrature)  │                  │
  2× Pots (Vol+Cutoff) ──►│ ADC1 (2 canales)         │                  │
  16× Switches mecán. ───►│ MCP23017 I2C 0x20 INT    │                  │
  ~12× Botones función ──►│ MCP23017 I2C 0x20        │                  │
                        │ PCA9685 I2C 0x40──►16× LED 0603 SMD      │
                        │                          ▼                  │
                        │              ┌─────────────────┐            │
                        │              │  Motor de        │            │
                        │              │  Síntesis FM     │            │
                        │              │  4–6 operadores  │            │
                        │              │  @44100Hz        │            │
                        │              └────────┬────────┘            │
                        │                       │                     │
                        │              ┌────────▼────────┐            │
                        │              │  Motor ADSR      │            │
                        │              │  4 voces ×       │            │
                        │              │  3 destinos      │            │
                        │              └──┬──────────┬───┘            │
                        │                 │          │                │
                        │  SAI1 (I2S) ◄───┘          │ SPI1           │
                        │                            ▼                │
                        │                       DAC8564               │
                        └────────┬───────────────────┬────────────────┘
                                 │                   │
                        ┌────────▼───────┐  ┌────────▼───────────────┐
                        │  PCM5242       │  │  CV Outputs (4 canales)│
                        │  DAC I2S 32bit │  │  VCA CV / VCF CV       │
                        │  112dB DNR     │  │  Pitch CV / Aux CV     │
                        └────────┬───────┘  └────────────────────────┘
                                 │ Audio digital                       
                                 ▼                                    
                        ┌────────────────┐                            
                        │  AS3320 VCF    │◄── CV Cutoff (de DAC8564)  
                        │  Moog Ladder   │◄── CV Resonance            
                        │  -24dB/Oct     │◄── Audio IN Externo        
                        └────────┬───────┘                            
                                 │                                    
                        ┌────────▼───────┐                            
                        │  THAT2180 VCA  │◄── CV Amplitud (de DAC8564)
                        │  >120dB rango  │                            
                        └────────┬───────┘                            
                                 │                                    
                        ┌────────▼───────┐                            
                        │  OPA2134 Buffer│                            
                        │  Salida estéreo│                            
                        └────────┬───────┘                            
                                 │                                    
                    Jack TRS L/R + Jack XLR (via transformador)       
```

---

## 2. Sección Digital — Detalles y Complicaciones

### 2.1 STM32H7 — Configuración del Clock Tree

El STM32H743 tiene uno de los árboles de clock más complejos del mercado ARM Cortex-M.
**Principal riesgo:** configurar mal el PLL y obtener 80MHz en lugar de 480MHz sin saberlo.

```
HSE (cristal 25MHz)
  └─► PLL1: M=5, N=192, P=2 ──► 480MHz (SYSCLK, CPU)
           N=192, Q=4 ──► 240MHz (USB? NO — USB requiere 48MHz exacto)
  └─► PLL3: M=5, N=48,  P=1 ──► 48MHz exacto para USB OTG FS
           N=48,  Q=5  ──► 48MHz para SAI1 (audio word clock @ 48kHz)
```

**Complicación crítica:** USB FS **requiere exactamente 48MHz** en su clock.
Si el PLL no genera 48MHz exactos, el host USB rechazará el dispositivo.
La solución es usar un cristal de 12MHz o 25MHz con los divisores correctos.
Con 25MHz: M=5, N=96, P=2 → 25/5 × 96 / 2 = **240MHz** ← error clásico.
Con 25MHz: M=5, N=192, P=20 → 25/5 × 192 / 20 = **48MHz exacto** ← correcto para USB.

### 2.2 STM32H7 — Gestión de RAM

El H743 tiene múltiples bancos de RAM en diferentes dominios:
```
DTCM  (512KB): Acceso directo desde CPU, SIN DMA. Usar para tablas de ondas.
AXI SRAM (512KB): Accesible por DMA. Usar para buffers de audio I2S.
SRAM1–3 (288KB): Usar para datos de presets y UI.
Backup SRAM (4KB): Persiste durante reset. Usar para último preset activo.
```

**Error frecuente:** Poner el buffer de audio DMA en DTCM → el DMA no puede acceder
a DTCM → silencio total sin mensajes de error. Solución: atributo de sección `__attribute__((section(".dma_buffer")))` en SRAM AXI.

### 2.3 Síntesis FM — Algoritmos y Carga de CPU

La síntesis FM de 6 operadores a 48000Hz en 4 voces simultáneas:
```
Carga estimada: 6 operadores × 4 voces × 44100 Hz
= 1,152,000 operaciones de punto flotante por segundo

STM32H743 FPU: ~240 MFLOPS (single precision)
Carga porcentual: ~0.44% del CPU ← cómodo
```

La DSP de reverb (Schroeder o FDN de 8 líneas de delay) es lo más pesado:
```
Reverb FDN-8 a 44100Hz estéreo: ~8 × delay_líneas × 44100 = ~3.5 MFLOPS
Carga porcentual: ~1.5% del CPU ← aún manejable
```

**Complicación:** La síntesis FM con muchos operadores y retroalimentación (feedback)
puede generar ruido DC que se acumula en el DAC de audio. Solución: filtro DC-blocking
por software (highpass a 20Hz: `y = 0.9997 × (y + x - x_prev)`).

---

## 3. Sección Analógica — Detalles y Complicaciones

### 3.1 AS3320 VCF — El Circuito Más Crítico

El AS3320 es un clon del CEM3320 original del Moog. El circuito de bias es delicado.

**Schematic mínimo funcional:**
```
Audio IN ──[1kΩ]──► Pin 2 (IN+)   AS3320
Audio IN ──[1kΩ]──► Pin 3 (IN-)              ► Pin 14 (LP4 OUT) ──[10kΩ]──► Post-VCA
GND ──[1kΩ]──► Pin 2 también (suma a tierra)
VCF CV (0–5V) ──► [V/I conversor con NPN 2N3904] ──► Pin 13 (ICTL)
Resonance CV ──► [V/I conversor] ──► Pin 9 (IRES)
+12V, -12V alimentación ± con 100nF desacoplo en cada pin
```

**Complicaciones conocidas del AS3320:**

| Problema | Causa | Solución |
|---|---|---|
| Pitch se desvía con temperatura | El transistor interno tiene coeficiente térmico | Resistor de compensación de temperatura (tempco) 3300ppm/°C en serie con ICTL |
| Ruido en frecuencias altas | Tierra de señal compartida con tierra digital | Star ground estricto, AGND físicamente separado |
| Resonancia inestable / oscilaciones incontroladas | IRES mal calibrado | Trimpot de 10kΩ para ajuste fino, limitar el rango del CV |
| Distorsión a frecuencias bajas con nivel alto | Saturación de la etapa de entrada | Atenuador de entrada: resistor de 1kΩ en cada entrada |
| Self-oscillation no es stable en pitch | Tracking V/oct no calibrado | Calibración con trimpot y nota de referencia |

### 3.2 Separación de Planos de Tierra — La Regla Más Importante

```
CORRECTO:                    INCORRECTO:
                             
AGND──┐              AGND────────────────── DGND
DGND──┘                  (lazo de tierra = antena de ruido)
      │
   Star Ground
   (un solo punto)
```

**Regla absoluta:** AGND y DGND se conectan físicamente **en un único punto** cerca del regulador
de voltaje analógico. En el layout de PCB, son planos separados que se tocan **solo ahí**.

**Consecuencia de no hacerlo:** Ruido de switching del STM32 (armonías de 480MHz y sus submúltiplos)
aparece directamente en la salida de audio. Perceptible como un zumbido o pitido digital de fondo.

### 3.3 THAT2180 VCA — Control Logarítmico vs Lineal

El THAT2180 tiene control en dB/Voltio (20mV/dB), lo que significa que su respuesta
es logarítmica en voltaje pero lineal en dB. Esto es **exactamente** lo que el oído necesita.

**Problema:** El DAC8564 produce una rampa de envolvente **lineal en voltaje**.
Si se alimenta directamente, el VCA suena como si el attack fuera muy lento al principio
y muy rápido al final — el comportamiento opuesto al percibido como natural.

**Solución (dos opciones):**

Opción A — Software: Convertir la envolvente a escala dB antes del DAC:
```c
// Convertir nivel lineal [0,1] a voltaje de control dB para THAT2180
// THAT2180: -0.02V por dB, rango útil -80dB a 0dB
float level_db = 20.0f * log10f(current_level + 1e-6f); // evitar log(0)
float control_v = level_db * (-0.02f);  // -0.02V/dB
uint16_t dac_word = voltage_to_dac16(control_v);
```

Opción B — Hardware: Convertidor antilog con transistor NPN en lazo de retroalimentación
de un op-amp (circuito estándar de control de VCA con ley exponencial).

**Recomendación:** Opción A — más flexible y permite cambiar la curva desde software.

---

## 4. Gestión de Energía — Aislamiento y Ruido

### 4.1 Esquema de Fuentes

```
USB-C (5V / 1.5A)
    │  CC1/CC2 ── 5.1kΩ a GND (señaliza 1.5A al cargador, sin PD controller IC)
    │  Polyfuse 2A (reseteable)
    │
    ├── ADP3335 (LDO 3.3V) ──► +3V3_DIG
    │       │                       │
    │    (ferritas en entrada)   STM32H7, lógica, USB, display, W25Q128
    │
    ├── LT3042 (LDO noise < 0.8µV RMS) ──► +5V_ANA @ 200mA
    │       │                                   │
    │    (ultra-bajo ruido)             AS3320 VDD, DAC8564 AVDD, PCM5242 AVDD
    │                                   OPA2134, NE5532 VCC, PCA9685 VDD
    │
    └── LT1054 (charge pump, DIP8) ──► -5.5V raw
              │
           LT3094 (LDO rail negativo, MSOP10) ──► -5V_ANA @ 100mA
                   │
               AS3320 VSS, THAT2180 VEE, op-amps VEE, NE5532 VEE
```

### 4.2 Ventaja de la Arquitectura Sin Switching

**No hay Buck converter en este diseño.** El USB-C proporciona 5V directamente.
Esto elimina la fuente principal de ruido de switching en la mayoría de los sintetizadores.

**Ventajas de esta arquitectura:**
1. **Sin ruido de switching** — no hay 2.2MHz de un Buck converter en el rail analógico
2. **LT3042 (PSRR > 76dB @ 100kHz)** — filtra cualquier ruido remanente del USB
3. **LT3094 complementario** — la misma arquitectura de bajo ruido para el rail negativo
4. **LT1054 (carga bomba)** — trabaja a ~100kHz pero está antes del LT3094; el LDO lo filtra

**Mitigación residual obligatoria:**
- Ferrita bead (100Ω @ 100MHz) entre USB VBUS y entrada de ADP3335/LT3042
- 100µF electrolítico + 100nF cerámico en entrada y salida de cada LDO
- Separación física: LT1054 al menos 15mm de cualquier circuito de audio

---

## 5. USB MIDI — Complicaciones de Compatibilidad

### 5.1 Windows y el problema del driver MIDI 1.0

Windows 10 (versiones hasta 2004) tiene un bug conocido: el driver USB MIDI Class 1.0 nativo
tiene latencia de hasta **100ms**. Solución: usar **USB MIDI 2.0** o instalar un driver de terceros
(loopMIDI + driver de Korg/MOTU) en Windows 10 antiguo.

Windows 11 con la actualización de Noviembre 2023 o posterior tiene soporte nativo de USB MIDI 2.0.

### 5.2 Protección contra cortocircuito en USB

El USB host (PC) puede suministrar hasta 500mA (USB 2.0) o 900mA (USB 3.0).
Si hay un cortocircuito en el diseño, se puede dañar el puerto USB del PC.

**Protección obligatoria:**
- Fusible reseteable Polyfuse (500mA) en la línea VBUS de USB
- Diodo TVS USBLC6-2 para protección ESD en D+ y D-
- El STM32H7 ya incluye resistores de pull-up en D+ internamente vía software

### 5.3 Descriptor USB tamaño total

El campo `wTotalLength` del Configuration Descriptor **debe ser exactamente correcto**.
Si es incorrecto por 1 byte, el dispositivo no será reconocido y Windows mostrará
"USB device not recognized" sin más detalle.

Herramienta para verificar: **USB View** (incluido en Windows SDK) o **USB Lyzer** (gratuito).

---

## 6. Interferencia Electromagnética (EMI) — El Problema Invisible

### 6.1 Fuentes de Ruido en el Diseño

| Fuente | Frecuencia | Impacto en audio |
|---|---|---|
| STM32H7 HCLK | 480MHz y armónicos | Ruido como tono agudo si hay coupling |
| USB FS clock | 12MHz y armónicos | Zumbido modulado |
| LT1054 (charge pump) | ~100kHz | Filtrado por LT3094 (40dB PSRR a 100kHz) antes del audio |
| SPI al DAC8564 | Hasta 50MHz | Glitches en CV si hay reflexión |
| PCA9685 I2C clock | 400kHz | Mínimo impacto con bus pull-ups correctos |

### 6.2 Reglas de Layout para Minimizar EMI

1. **Líneas diferenciales USB** (D+/D-): deben ser iguales en longitud, separadas de AGND, impedancia 90Ω diferencial
2. **Líneas de SPI al DAC8564**: el clock SPI no debe cruzar encima del plano AGND
3. **Pistas de audio** (analog): trazarlas por el lado del plano AGND, perpendiculares a las pistas digitales (no paralelas)
4. **Cristal**: colocar tan cerca del STM32H7 como sea posible, con plano de tierra bajo los pads del cristal
5. **Condensadores de bypass**: SIEMPRE del pad del IC hacia la via de tierra, no al revés

### 6.3 Cómo Detectar Ruido de EMI en el Diseño

1. Herramienta software: **REW (Room EQ Wizard)** + interfaz de audio → medir floor de ruido
2. Prueba básica: conectar salida del sintetizador a interfaz de audio, silencio de síntesis, medir espectro
3. Un pico a 480kHz (sub-armónico de 480MHz) indica coupling del clock principal
4. Un pico a ~100kHz puede indicar acoplamiento del LT1054 (charge pump)
5. Solución de campo: ferrita toroide en el cable de salida de audio (cura el síntoma, no la causa)

---

## 7. Testing y Validación

### 7.1 Equipamiento Mínimo para Pruebas

| Instrumento | Para qué | Costo aproximado |
|---|---|---|
| Osciloscopio 2 canales (mín. 100MHz) | Formas de onda, timing SPI/I2S | $200–400 USD (Rigol DS1054Z) |
| Multímetro de precisión | Voltajes DC, resistencias, diodos | $30–80 USD |
| Generador de señal de audio | Inyectar señal de prueba al VCF | $15 USB stick oscilador, o PC + cable |
| Interfaz de audio USB (24-bit) | Medir THD+N, floor de ruido | $60–150 USD (Focusrite Scarlett 2i2) |
| Lupa/microscopio de soldadura | Verificar soldaduras SMD | $30–80 USD (cámara USB con zoom) |
| Programador/debugger SWD | Flash y debug del STM32H7 | $15 USD (ST-Link V3 clone) |

### 7.2 Pruebas Críticas Antes de Encender por Primera Vez

```
LISTA DE VERIFICACIÓN PRE-ENCENDIDO (PCB nueva):
□ Medir resistencia VBUS_3V3 a GND: debe ser > 1kΩ (si es 0Ω = cortocircuito)
□ Medir resistencia +5V_ANA a GND: debe ser > 500Ω
□ Medir resistencia -5V_ANA a GND: debe ser > 500Ω
□ Verificar orientación de todos los electrolíticos con lupas
□ Verificar orientación del STM32H7 (punto de pin 1)
□ Verificar orientación del PCM5242 y DAC8564
□ Alimentar primero con fuente de laboratorio limitada a 100mA
□ Medir corrientes de entrada: si > 150mA en frío → hay cortocircuito
□ Medir voltajes bajo carga antes de conectar cualquier IC
```

---

## 8. Resumen de Riesgos del Proyecto

| Riesgo | Probabilidad | Impacto | Mitigación |
|---|---|---|---|
| PCB con error de diseño | Alta (primera revisión) | Alto | Revisar ERC/DRC exhaustivamente antes de enviar Gerbers |
| AS3320 dañado por voltaje | Media | Alto | Verificar ±5V_ANA antes de conectar (no ±12V) |
| Ruido de fuente en audio | Baja | Medio | Arquitectura sin Buck: USB-C → LT3042/LT3094 — ruido inherentemente bajo |
| USB no reconocido por Windows | Media | Medio | Validar descriptores con USB View antes de soldar |
| Deriva de temperatura en VCF | Alta | Bajo | Tempco resistor obligatorio, calibración post-montaje |
| STM32H7 dañado por ESD | Baja | Alto | Manejo con pulsera antiestática, PCB guardada en bolsa |
| Escasez de AS3320 | Media | Alto | Comprar 5–10 unidades en el primer pedido |
