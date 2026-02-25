# Lógica de UI — DigitalSynth v1.0
**Versión:** 1.0 | **Fecha:** 25 Feb 2026
**Referencia de diseño:** Elektron Digitone (simplificado)

---

## ÍNDICE

1. [Inventario de Controles Físicos](#1-inventario-de-controles-físicos)
2. [Mapa de Hardware — GPIO y MCP23017](#2-mapa-de-hardware--gpio-y-mcp23017)
3. [Páginas de Encoders](#3-páginas-de-encoders)
4. [Funciones de los 16 Botones de Función](#4-funciones-de-los-16-botones-de-función)
5. [Funciones de los 4 Botones de Track](#5-funciones-de-los-4-botones-de-track)
6. [Funciones de los 16 Step Buttons](#6-funciones-de-los-16-step-buttons)
7. [Sistema SHIFT — Funciones Secundarias](#7-sistema-shift--funciones-secundarias)
8. [LEDs SK6812 — Código de Color](#8-leds-sk6812--código-de-color)
9. [Máquina de Estados](#9-máquina-de-estados)
10. [Parámetros de Voz por Track](#10-parámetros-de-voz-por-track)
11. [Sistema de Patrones y Song Mode](#11-sistema-de-patrones-y-song-mode)

---

## 1. INVENTARIO DE CONTROLES FÍSICOS

| Elemento | Cantidad | Hardware | Notas |
|---|---|---|---|
| Encoders rotativos EC11 | 8 | STM32 TIM (ENC1-4) / EXTI (ENC5-8) | Con push integrado |
| Push de encoders | 8 | GPIO directo STM32 | PD5–PD11, PE0 |
| Step buttons (SW1–SW16) | 16 | MCP23017 U5 (0x20) | Con SK6812 RGB integrado |
| Track buttons (T1–T4) | 4 | GPIO directo STM32 | PD0, PD1, PD14, PD15 |
| Botones de función | 16 | MCP23017 U9 (0x21) | Grid 4×4 |
| Potenciómetro master vol | 1 | ADC PA0 | RV3 — 10kΩ lineal |
| Potenciómetro filter | 1 | ADC PA1 | RV4 — 10kΩ lineal |
| Salidas CV | 4 | DAC8565 SPI | CV1–CV4, 0–5V |
| Display | 1 | SSD1309 I2C 0x3C | 128×64, 2.42" |

---

## 2. MAPA DE HARDWARE — GPIO Y MCP23017

### 2.1 Track Buttons — GPIO directo STM32

> Los botones de track tienen la prioridad más alta del sistema (respuesta inmediata).
> Por eso van directamente al STM32, no al MCP23017.

| Botón | GPIO STM32 | Net KiCad | Pull-up | Color LED panel |
|---|---|---|---|---|
| T1 (Track 1) | PD0 | BTN_TRACK1 | Interno + 100nF | Rojo |
| T2 (Track 2) | PD1 | BTN_TRACK2 | Interno + 100nF | Amarillo |
| T3 (Track 3) | PD14 | BTN_TRACK3 | Interno + 100nF | Verde |
| T4 (Track 4) | PD15 | BTN_TRACK4 | Interno + 100nF | Azul |

**En CubeMX:** `GPIO_Input → Pull-up`, opcionalmente `EXTI Falling Edge` para ISR.

---

### 2.2 Step Buttons + SK6812 — MCP23017 U5 (dirección 0x20)

| Pin MCP23017 U5 | Net | Switch físico | LED SK6812 |
|---|---|---|---|
| GPA0 | SW1 | Step 1 | RGB integrado SW1 |
| GPA1 | SW2 | Step 2 | RGB integrado SW2 |
| GPA2 | SW3 | Step 3 | RGB integrado SW3 |
| GPA3 | SW4 | Step 4 | RGB integrado SW4 |
| GPA4 | SW5 | Step 5 | RGB integrado SW5 |
| GPA5 | SW6 | Step 6 | RGB integrado SW6 |
| GPA6 | SW7 | Step 7 | RGB integrado SW7 |
| GPA7 | SW8 | Step 8 | RGB integrado SW8 |
| GPB0 | SW9 | Step 9 | RGB integrado SW9 |
| GPB1 | SW10 | Step 10 | RGB integrado SW10 |
| GPB2 | SW11 | Step 11 | RGB integrado SW11 |
| GPB3 | SW12 | Step 12 | RGB integrado SW12 |
| GPB4 | SW13 | Step 13 | RGB integrado SW13 |
| GPB5 | SW14 | Step 14 | RGB integrado SW14 |
| GPB6 | SW15 | Step 15 | RGB integrado SW15 |
| GPB7 | SW16 | Step 16 | RGB integrado SW16 |

**INTA de U5** → PC0 (GPIO_EXTI, activo LOW, pull-up)
**RESET de U5** → pull-up 10kΩ a +3.3V (no necesita GPIO, siempre activo)
**Dirección:** A2=A1=A0=GND → 0x20

**Registro IOCON:** Configurar `MIRROR=1` → INTA refleja cambios en GPA y GPB.
Los 16 pines como INPUT con pull-up. El switch lleva el pin a GND al presionar.

---

### 2.3 Botones de Función — MCP23017 U9 (dirección 0x21)

| Pin MCP23017 U9 | Net | Botón | Fila |
|---|---|---|---|
| GPA0 | BTN_TRACK_PAGE | TRACK (página) | Fila A |
| GPA1 | BTN_SEQ_PAGE | SEQ (página) | Fila A |
| GPA2 | BTN_MOD_PAGE | MOD (página) | Fila A |
| GPA3 | BTN_SONG_PAGE | SONG (página) | Fila A |
| GPA4 | BTN_PLAY | PLAY ▶ | Fila B |
| GPA5 | BTN_STOP | STOP ■ | Fila B |
| GPA6 | BTN_REC | REC ● | Fila B |
| GPA7 | BTN_TAP | TAP | Fila B |
| GPB0 | BTN_PATT_PREV | PATT ◄ | Fila C |
| GPB1 | BTN_PATT_NEXT | PATT ► | Fila C |
| GPB2 | BTN_CHAIN | CHAIN | Fila C |
| GPB3 | BTN_FILL | FILL | Fila C |
| GPB4 | BTN_SHIFT | SHIFT | Fila D |
| GPB5 | BTN_COPY | COPY | Fila D |
| GPB6 | BTN_PASTE | PASTE | Fila D |
| GPB7 | BTN_CLEAR | CLEAR | Fila D |

**INTA de U9** → PC1 (GPIO_EXTI, activo LOW, pull-up)
**Dirección:** A0=+3.3V, A1=A2=GND → 0x21

---

### 2.4 Encoders Rotativos — STM32

#### Encoders Hardware (TIM quadrature)

| Encoder | Pin A | Pin B | Push | Timer | Etiqueta funcional |
|---|---|---|---|---|---|
| ENC1 | PA15 | PB3 | PD5 | TIM2 | Parámetro 1 |
| ENC2 | PC6 | PC7 | PD6 | TIM3 | Parámetro 2 |
| ENC3 | PD12 | PD13 | PD7 | TIM4 | Parámetro 3 |
| ENC4 | PE9 | PE11 | PD8 | TIM1 | Parámetro 4 |

#### Encoders Software (EXTI)

| Encoder | Pin A (EXTI) | Pin B (poll) | Push | Etiqueta funcional |
|---|---|---|---|---|
| ENC5 | PC8 | PC9 | PD9 | Parámetro 5 |
| ENC6 | PC10 | PC11 | PD10 | Parámetro 6 |
| ENC7 | PC12 | PD2 | PD11 | Parámetro 7 |
| ENC8 | PD3 | PD4 | PE0 | Parámetro 8 |

---

## 3. PÁGINAS DE ENCODERS

Los 8 encoders tienen funciones distintas según la **página activa**. La página se selecciona con los botones TRACK / SEQ / MOD / SONG.

El display siempre muestra el nombre y valor de los 8 parámetros de la página actual en una grilla 4+4.

---

### PÁGINA: TRACK — Parámetros de la voz del track activo

| Encoder | Giro | Push | Rango | Descripción |
|---|---|---|---|---|
| ENC1 | Nota base | Reset a C | C0–B7 | Nota raíz del track (offset de la secuencia) |
| ENC2 | Octava | Reset a 3 | 0–7 | Transposición de octava del track |
| ENC3 | Portamento | On/Off toggle | 0–1000ms | Tiempo de glide entre notas. Push activa/desactiva |
| ENC4 | Rango CV | Toggle 2.5V/5V | 0–5V | Gain del DAC8565 — 2.5V = 2.5 oct, 5V = 5 oct |
| ENC5 | Gate length | Reset a 50% | 1%–100% | Duración del gate como % del step length |
| ENC6 | Nivel CV | Reset a 100% | 0–100% | Atenuación de la salida CV (escala el rango) |
| ENC7 | Accent | Reset a 0 | 0–100% | Voltaje extra en steps marcados como accent (+V) |
| ENC8 | Bend range | Reset a 0 | 0–24 st | Rango del bend en semitonos |

---

### PÁGINA: SEQ — Configuración del secuenciador del track activo

| Encoder | Giro | Push | Rango | Descripción |
|---|---|---|---|---|
| ENC1 | Longitud patrón | Reset a 16 | 1–16 steps | Cuántos steps tiene el loop del track |
| ENC2 | Escala musical | Toggle | Cromática/Mayor/Menor/Penta... | Cuantiza la entrada de notas a una escala |
| ENC3 | Nota raíz de escala | Reset a C | C–B | Tónica del la escala seleccionada |
| ENC4 | Dirección | Toggle | →/←/↔/random/pendulum | Dirección de reproducción del patrón |
| ENC5 | Tempo (BPM) | Tap tempo | 30–300 BPM | Tempo global del proyecto |
| ENC6 | Swing | Reset a 50% | 50%–75% | Porcentaje de swing aplicado a los steps pares |
| ENC7 | Multiplicador | Toggle | ÷4/÷2/×1/×2/×4 | Velocidad relativa del track respecto al tempo master |
| ENC8 | Probabilidad global | Reset a 100% | 0%–100% | Probabilidad de que cada step suene |

---

### PÁGINA: MOD — Modulación (2 LFOs por track)

| Encoder | Giro | Push | Rango | Descripción |
|---|---|---|---|---|
| ENC1 | LFO1 Rate | Sync on/off | 0.01–500 Hz | Velocidad del LFO1. Push sinc al tempo |
| ENC2 | LFO1 Shape | Cycle through | Sine/Tri/Saw/Ramp/Square/SH | Forma de onda del LFO1 |
| ENC3 | LFO1 Depth | Reset a 0 | -100%–+100% | Profundidad de modulación del LFO1 |
| ENC4 | LFO1 Destino | Cycle through | Pitch/CV Level/Gate/Accent | Parámetro modulado por LFO1 |
| ENC5 | LFO2 Rate | Sync on/off | 0.01–500 Hz | Velocidad del LFO2 |
| ENC6 | LFO2 Shape | Cycle through | Sine/Tri/Saw/Ramp/Square/SH | Forma de onda del LFO2 |
| ENC7 | LFO2 Depth | Reset a 0 | -100%–+100% | Profundidad de modulación del LFO2 |
| ENC8 | LFO2 Destino | Cycle through | Pitch/CV Level/Gate/Accent | Parámetro modulado por LFO2 |

> **Sample & Hold (SH):** El LFO toma un valor aleatorio nuevo en cada step del secuenciador.
> Útil para modulación aleatoria sincronizada al tempo.

---

### PÁGINA: SONG — Gestión de patrones y proyecto

| Encoder | Giro | Push | Rango | Descripción |
|---|---|---|---|---|
| ENC1 | Patrón actual | — | 1–128 | Selecciona el patrón a editar/reproducir |
| ENC2 | Modo de reproducción | Toggle | Loop/One-shot/Ping-pong | Comportamiento del patrón al terminar |
| ENC3 | Bank | Toggle A/B/C/D | A/B/C/D | Banco de 32 patrones activo (4 bancos × 32 = 128) |
| ENC4 | Patrón destino | — | 1–128 | Destino para operaciones COPY/PASTE/MOVE |
| ENC5 | Tempo master | Push click | 30–300 BPM | Mismo parámetro que en página SEQ |
| ENC6 | Volumen master | Reset a 100% | 0–100% | Igual al potenciómetro RV3 pero en software |
| ENC7 | Song chain index | — | 1–64 | Posición en la lista de encadenado de patrones |
| ENC8 | Slot de guardado | Confirm (hold 1s) | 1–8 | Ranura de proyecto. Hold para guardar a Flash |

---

## 4. FUNCIONES DE LOS 16 BOTONES DE FUNCIÓN

### Layout físico (Grid 4×4)

```
┌──────────┬──────────┬──────────┬──────────┐
│  TRACK   │   SEQ    │   MOD    │   SONG   │  ← Fila A: Páginas
├──────────┼──────────┼──────────┼──────────┤
│  PLAY ▶  │  STOP ■  │  REC ●   │   TAP    │  ← Fila B: Transporte
├──────────┼──────────┼──────────┼──────────┤
│  PATT◄   │  PATT►   │  CHAIN   │   FILL   │  ← Fila C: Patrones
├──────────┼──────────┼──────────┼──────────┤
│  SHIFT   │   COPY   │  PASTE   │  CLEAR   │  ← Fila D: Edición
└──────────┴──────────┴──────────┴──────────┘
```

### Fila A — Páginas de encoders

| Botón | Función primaria | LED cuando activo |
|---|---|---|
| TRACK | Activa página TRACK (parámetros de voz) | Encendido sólido |
| SEQ | Activa página SEQ (secuenciador) | Encendido sólido |
| MOD | Activa página MOD (LFOs) | Encendido sólido |
| SONG | Activa página SONG (patrones y proyecto) | Encendido sólido |

> Solo una página puede estar activa. Pulsar la misma página activa no hace nada.

### Fila B — Transporte

| Botón | Función primaria | Comportamiento detallado |
|---|---|---|
| PLAY ▶ | Start/Continue | Si está parado: arranca desde la posición actual. Si está corriendo: no hace nada |
| STOP ■ | Stop | Primera pulsación: para en el step actual. Segunda pulsación: vuelve al step 1 |
| REC ● | Record mode | Activa grabación en tiempo real — los steps tocados o programados se guardan |
| TAP | Tap tempo | Cada pulsación mide el intervalo. Promedio de las últimas 4 pulsaciones = BPM |

### Fila C — Gestión de patrones

| Botón | Función primaria | Comportamiento detallado |
|---|---|---|
| PATT ◄ | Patrón anterior | Cambia al patrón previo. El cambio ocurre al final del patrón actual (cuantizado) |
| PATT ► | Patrón siguiente | Cambia al patrón siguiente. Cuantizado al final del patrón actual |
| CHAIN | Encadenado de patrones | Activa el modo Song — reproduce la lista de patrones encadenados en orden |
| FILL | Fill | Mientras está presionado, activa el pattern de fill de cada track |

### Fila D — Edición

| Botón | Función primaria | Comportamiento detallado |
|---|---|---|
| SHIFT | Modificador global | No tiene función propia — activa funciones secundarias de otros botones |
| COPY | Copiar | Copia el patrón del track activo al buffer temporal |
| PASTE | Pegar | Pega el buffer temporal en el track/patrón activo |
| CLEAR | Limpiar | Borra los steps del patrón activo después de confirmación (hold 1s) |

---

## 5. FUNCIONES DE LOS 4 BOTONES DE TRACK

```
[ T1 ]  [ T2 ]  [ T3 ]  [ T4 ]
  CV1     CV2     CV3     CV4
```

| Botón | Función primaria | Hold 500ms | Con SHIFT |
|---|---|---|---|
| T1 | Selecciona Track 1 activo | Mute/Unmute Track 1 | Solo Track 1 (mutea T2/T3/T4) |
| T2 | Selecciona Track 2 activo | Mute/Unmute Track 2 | Solo Track 2 |
| T3 | Selecciona Track 3 activo | Mute/Unmute Track 3 | Solo Track 3 |
| T4 | Selecciona Track 4 activo | Mute/Unmute Track 4 | Solo Track 4 |

**Indicadores LED del panel de track:**
- LED encendido constante = track seleccionado para edición
- LED parpadeando lento (1Hz) = track muteado
- LED apagado = track activo pero no seleccionado
- LED parpadeando rápido (4Hz) = track en solo

> El mute es "suave" — el secuenciador sigue corriendo internamente.
> Al desactivar el mute, el track se sincroniza al step actual (no hay out-of-sync).

---

## 6. FUNCIONES DE LOS 16 STEP BUTTONS

Los step buttons tienen múltiples roles según el contexto:

### Modo NORMAL (reproducción/edición de pasos)

| Acción | Resultado |
|---|---|
| Tap breve en step vacío | Activa el step (pone nota base del track) |
| Tap breve en step activo | Desactiva el step |
| Hold step (> 400ms) | Entra en modo STEP EDIT — display muestra parámetros del step |
| Hold step + girar ENC1 | Cambia la nota del step (semitono) |
| Hold step + girar ENC2 | Cambia la octava del step |
| Hold step + girar ENC3 | Cambia el portamento individual del step |
| Hold step + girar ENC4 | Cambia la velocidad/nivel CV del step |
| Hold step + girar ENC5 | Cambia el gate length del step |
| Hold step + girar ENC7 | Cambia el accent del step |
| Hold step + girar ENC8 | Cambia la probabilidad individual del step (0–100%) |

### Modo NOTE INPUT (entrada de notas — activado por REC)

Cuando REC está activo y la página es SEQ:

| Step | Nota en escala cromática |
|---|---|
| SW1 | Nota raíz (C en escala por defecto) |
| SW2 | C# / Db |
| SW3 | D |
| ... | Sigue la escala configurada |
| SW13–SW16 | Notas de la octava superior |

### Modo TRIG CONDITIONS (SHIFT + hold step)

| Combinación | Función |
|---|---|
| SHIFT + hold step + ENC8 | Probabilidad: 0%, 12.5%, 25%, 50%, 75%, 87.5%, 100% |
| SHIFT + hold step + ENC1 | Condición: Free / 1st / Fill / NotFill / Neighbor / Pre |
| SHIFT + hold step + ENC2 | Ratio: 1:2 / 1:3 / 1:4 / 2:3 / etc. (suena cada N repeticiones) |

**Condiciones de trig disponibles:**
- `Free` — siempre suena (por defecto)
- `1st` — solo suena la primera vez que pasa el loop
- `Fill` — solo suena cuando FILL está activo
- `NotFill` — suena siempre excepto cuando FILL está activo
- `Pre` — pre-trigge: prepara el note antes del step
- `1:2`, `2:3`, etc. — suena en ratio con el número de loops

---

## 7. SISTEMA SHIFT — FUNCIONES SECUNDARIAS

### SHIFT + Botones de página

| Combinación | Función |
|---|---|
| SHIFT + TRACK | Sub-página: configuración de CV routing (qué módulo recibe qué CV) |
| SHIFT + SEQ | Sub-página: configuración de escala global del proyecto |
| SHIFT + MOD | Sub-página: configuración de envelope (si se implementa) |
| SHIFT + SONG | Sub-página: configuración del proyecto (BPM, compás, etc.) |

### SHIFT + Transporte

| Combinación | Función |
|---|---|
| SHIFT + PLAY | Restart — vuelve al step 1 de todos los tracks simultáneamente y arranca |
| SHIFT + STOP | Panic — envía todos los CV a 0V instantáneamente. Útil para silenciar synths colgados |
| SHIFT + REC | Overdub — graba encima sin borrar los steps existentes |
| SHIFT + TAP | Abre la pantalla de ajuste de BPM con ingreso numérico por encoders |

### SHIFT + Patrones

| Combinación | Función |
|---|---|
| SHIFT + PATT ◄ | Ir directamente al Bank anterior (A→D→C→B→A) |
| SHIFT + PATT ► | Ir directamente al Bank siguiente (A→B→C→D→A) |
| SHIFT + CHAIN | Editar la lista de song chain (añadir/quitar patrones del orden) |
| SHIFT + FILL | Fill permanente — queda activo hasta volver a presionar SHIFT+FILL |

### SHIFT + Edición

| Combinación | Función |
|---|---|
| SHIFT + COPY | Copiar todos los tracks del patrón (no solo el activo) |
| SHIFT + PASTE | Paste selectivo — display pregunta qué tracks pegar |
| SHIFT + CLEAR | Clear del patrón completo (todos los tracks) |

### SHIFT + Track buttons

| Combinación | Función |
|---|---|
| SHIFT + T1 | Solo Track 1 — mutea T2/T3/T4 |
| SHIFT + T2 | Solo Track 2 |
| SHIFT + T3 | Solo Track 3 |
| SHIFT + T4 | Solo Track 4 |
| SHIFT + T1 + T2 | Solo Tracks 1+2 |
| SHIFT + cualquier step | Accent en ese step (toggle) |

### SHIFT + ENC push

| Combinación | Función |
|---|---|
| SHIFT + ENC8 push | Undo — revierte el último cambio de parámetro |
| SHIFT + ENC1 push | Randomize el parámetro 1 del track activo |
| SHIFT + PLAY (hold 2s) | Guarda el proyecto en el slot activo de Flash |

---

## 8. LEDs SK6812 — CÓDIGO DE COLOR

Los 16 step buttons tienen LEDs RGB SK6812 integrados. El color comunica el estado:

### States del step

| Estado | Color | Valor GRB | Descripción |
|---|---|---|---|
| Vacío (sin nota) | Rojo muy tenue | `0x001400` | El step existe pero no tiene nota |
| Activo — Track 1 | Rojo | `0x00CC00` | Nota programada, track 1 |
| Activo — Track 2 | Amarillo | `0xCCCC00` | Nota programada, track 2 |
| Activo — Track 3 | Verde | `0xCC0000` | Nota programada, track 3 |
| Activo — Track 4 | Cyan/Azul | `0x0000CC` | Nota programada, track 4 |
| Cursor de play | Blanco | `0x808080` | Posición actual del secuenciador |
| Step muteado | Naranja parpadeante | `0x201000` ↔ off | Step desactivado |
| Step seleccionado | Magenta | `0xCC00CC` | Hold sobre este step (editando) |
| Accent activo | Color × 1.5 brillo | — | Velocidad/CV extra |
| Fill activo | Cicla colores | — | Animación durante fill |
| Step con probabilidad < 100% | Color + parpadeo lento | — | Indica que es probabilístico |

### Animaciones de sistema

| Situación | Animación | Descripción |
|---|---|---|
| Boot | Sweep de izquierda a derecha, blanco | Power-on sequence (0.5s) |
| Guardado exitoso | Todos parpadean verde 2× | Confirmación de save |
| Error (ej. Flash llena) | Todos parpadean rojo 3× | Error de operación |
| Panic | Todos off instantáneamente | SHIFT + STOP |
| Modo REC activo | Borde exterior (SW1/4/13/16) rojo pulsante | Indica modo grabación |

---

## 9. MÁQUINA DE ESTADOS

### Estados principales del sistema

```
┌─────────────────────────────────────────────────────────┐
│                    ESTADOS GLOBALES                     │
├─────────────┬─────────────┬─────────────┬──────────────┤
│   STOPPED   │   PLAYING   │  RECORDING  │    EDITING   │
│             │             │             │              │
│ - Seq parado│ - Seq corre │ - Playing + │ - Stopped +  │
│ - Edición   │ - Edición   │   graba     │   step hold  │
│   libre     │   en vivo   │   steps     │   activo     │
└─────────────┴─────────────┴─────────────┴──────────────┘
```

### Transiciones de estado

| Desde | Evento | Hasta |
|---|---|---|
| STOPPED | PLAY | PLAYING |
| PLAYING | STOP (1×) | PLAYING (pause en step actual) |
| PLAYING | STOP (2×) | STOPPED (vuelve a step 1) |
| PLAYING | REC | RECORDING |
| RECORDING | REC (toggle) | PLAYING |
| RECORDING | STOP | STOPPED |
| cualquiera | SHIFT+STOP | STOPPED + Panic CV=0 |
| cualquiera | Hold step | EDITING (overlay) |
| EDITING | Soltar step | Estado anterior |

### Sub-estados de página

```
PÁGINA ACTIVA: TRACK | SEQ | MOD | SONG
     ↑                              ↑
  botones Fila A              definen qué
                              muestran ENC1-8
```

---

## 10. PARÁMETROS DE VOZ POR TRACK

Cada uno de los 4 tracks almacena un estado completo independiente:

### Parámetros de secuenciador (por track)

```c
typedef struct {
    uint8_t  length;           // 1–16: longitud del patrón
    uint8_t  direction;        // 0=→, 1=←, 2=↔, 3=random, 4=pendulum
    uint8_t  multiplier;       // 0=÷4, 1=÷2, 2=×1, 3=×2, 4=×4
    uint8_t  scale;            // 0=chromatic, 1=major, 2=minor, 3=pentatonic...
    uint8_t  root_note;        // 0=C, 1=C#, 2=D...11=B
    uint8_t  swing;            // 50–75 (%)
    uint8_t  global_prob;      // 0–100 (%)
    uint8_t  muted;            // 0=active, 1=muted
} TrackSeq_t;
```

### Parámetros de voz CV (por track)

```c
typedef struct {
    uint8_t  base_note;        // 0–127 (MIDI note number)
    uint8_t  octave;           // 0–7
    uint16_t portamento_ms;    // 0–1000ms
    uint8_t  cv_range;         // 0=2.5V (2.5oct), 1=5V (5oct)
    uint8_t  cv_level;         // 0–100%
    uint8_t  gate_length;      // 1–100%
    uint8_t  accent_amount;    // 0–100%
    uint8_t  bend_range;       // 0–24 semitonos
} TrackVoice_t;
```

### Parámetros de step (por step, por track)

```c
typedef struct {
    uint8_t  active;           // 0=vacío, 1=activo
    uint8_t  note;             // 0–127 (offset desde base_note)
    uint8_t  octave_offset;    // -3 a +3
    uint8_t  gate_length;      // 1–100% (override de track si != 0)
    uint8_t  velocity;         // 0–127 (nivel CV)
    uint8_t  accent;           // 0=normal, 1=accent
    uint8_t  probability;      // 0–100%
    uint8_t  condition;        // 0=Free, 1=1st, 2=Fill, 3=NotFill, 4=Pre
    uint8_t  condition_ratio;  // numerador para ratios (1:2, 1:3, etc.)
    uint8_t  micro_timing;     // -12 a +12 (en ticks de resolución)
    uint8_t  slide;            // 0=off, 1=on (portamento hacia el siguiente step)
} Step_t;
```

### Parámetros LFO (2 por track)

```c
typedef struct {
    float    rate;             // 0.01–500 Hz
    uint8_t  shape;            // 0=Sine, 1=Tri, 2=Saw, 3=Ramp, 4=Square, 5=SH
    int8_t   depth;            // -100 a +100%
    uint8_t  destination;      // 0=Pitch, 1=CV Level, 2=Gate, 3=Accent
    uint8_t  sync_to_tempo;    // 0=free, 1=sincronizado al BPM
    uint8_t  sync_divider;     // divisor de tempo: 1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8
} LFO_t;
```

---

## 11. SISTEMA DE PATRONES Y SONG MODE

### Estructura de memoria en W25Q128 (16MB Flash)

```
Dirección    Contenido
0x000000     Header del proyecto (nombre, BPM, firma)
0x000100     Global settings (escala, template, etc.)
0x001000     Patrón 1  (4 tracks × 16 steps × 11 bytes/step = ~704 bytes)
0x001400     Patrón 2
...
0x???        Patrón 128
0x???        Song chain list (64 entradas)
0x???        Proyectos 2–8 (slots adicionales)
```

### Song Mode — Encadenado de patrones

La lista de song chain es una secuencia de hasta **64 entradas**, cada una especifica:
- **Patrón** a reproducir (1–128)
- **Repeticiones** (1–32 veces)
- **Track mutes** por entrada (qué tracks suenan en esa sección)

Ejemplo de song chain:
```
Entrada 1: Patrón 1 × 4 repeticiones  (intro)
Entrada 2: Patrón 5 × 8 repeticiones  (verso)
Entrada 3: Patrón 9 × 4 repeticiones  (coro)
Entrada 4: Patrón 5 × 4 repeticiones  (verso)
Entrada 5: Patrón 9 × 8 repeticiones  (coro final)
→ FIN
```

### Bancos de patrones

```
Bank A: Patrones 1–32    (ej. intro y verso)
Bank B: Patrones 33–64   (ej. coro y bridge)
Bank C: Patrones 65–96   (ej. variaciones)
Bank D: Patrones 97–128  (ej. fills y transiciones)
```

---

## HISTORIAL DE CAMBIOS

| Versión | Fecha | Descripción |
|---|---|---|
| 1.0 | 25 Feb 2026 | Versión inicial — lógica UI completa v1 |
