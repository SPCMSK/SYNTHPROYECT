# DigitalSynth v1.0 — Manual de Usuario
**Versión del manual:** 1.0 | **Fecha:** 25 Feb 2026
**Estado:** Borrador de trabajo — se actualiza conforme evoluciona el firmware

---

> Este manual describe el funcionamiento del DigitalSynth v1.0, un secuenciador
> CV/Gate de 4 canales con 16 pasos por track. El diseño está inspirado en el
> workflow de los sintetizadores Elektron, simplificado para una experiencia
> directa y musical.

---

## ÍNDICE

1. [Panel de Control](#1-panel-de-control)
2. [Primeros Pasos](#2-primeros-pasos)
3. [Conceptos Fundamentales](#3-conceptos-fundamentales)
4. [Tracks y Salidas CV](#4-tracks-y-salidas-cv)
5. [Programar una Secuencia](#5-programar-una-secuencia)
6. [Editar Parámetros de un Step](#6-editar-parámetros-de-un-step)
7. [Página TRACK — Voz y CV](#7-página-track--voz-y-cv)
8. [Página SEQ — Secuenciador](#8-página-seq--secuenciador)
9. [Página MOD — Modulación LFO](#9-página-mod--modulación-lfo)
10. [Página SONG — Patrones y Proyecto](#10-página-song--patrones-y-proyecto)
11. [Transporte y Grabación](#11-transporte-y-grabación)
12. [Gestión de Patrones](#12-gestión-de-patrones)
13. [Song Mode — Encadenado de Patrones](#13-song-mode--encadenado-de-patrones)
14. [FILL — Variaciones en Vivo](#14-fill--variaciones-en-vivo)
15. [Sistema SHIFT — Funciones Avanzadas](#15-sistema-shift--funciones-avanzadas)
16. [Trig Conditions — Probabilidad y Condiciones](#16-trig-conditions--probabilidad-y-condiciones)
17. [Guardar y Cargar Proyectos](#17-guardar-y-cargar-proyectos)
18. [Referencia Rápida de Botones](#18-referencia-rápida-de-botones)

---

## 1. PANEL DE CONTROL

```
╔══════════════════════════════════════════════════════════════════════╗
║                        DigitalSynth  v1.0                          ║
║                                                                      ║
║  ┌─────────────────────────────────────────────────────────┐        ║
║  │                     DISPLAY (2.42")                     │        ║
║  └─────────────────────────────────────────────────────────┘        ║
║                                                                      ║
║  ( ENC1 ) ( ENC2 ) ( ENC3 ) ( ENC4 ) ( ENC5 ) ( ENC6 ) ( ENC7 ) ( ENC8 )
║                                                                      ║
║  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐                               ║
║  │TRACK │ │ SEQ  │ │ MOD  │ │ SONG │    ← PÁGINAS                  ║
║  └──────┘ └──────┘ └──────┘ └──────┘                               ║
║  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐                               ║
║  │PLAY ▶│ │STOP ■│ │ REC ●│ │ TAP  │    ← TRANSPORTE               ║
║  └──────┘ └──────┘ └──────┘ └──────┘                               ║
║  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐                               ║
║  │PATT◄ │ │PATT► │ │CHAIN │ │ FILL │    ← PATRONES                 ║
║  └──────┘ └──────┘ └──────┘ └──────┘                               ║
║  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐                               ║
║  │SHIFT │ │ COPY │ │PASTE │ │CLEAR │    ← EDICIÓN                  ║
║  └──────┘ └──────┘ └──────┘ └──────┘                               ║
║                                                                      ║
║  ║SW1║ ║SW2║ ║SW3║ ║SW4║ ║SW5║ ║SW6║ ║SW7║ ║SW8║  ← PASOS 1–8    ║
║  ║SW9║ ║S10║ ║S11║ ║S12║ ║S13║ ║S14║ ║S15║ ║S16║  ← PASOS 9–16   ║
║                                                                      ║
║  [ T1 ]  [ T2 ]  [ T3 ]  [ T4 ]     (RV3) Master  (RV4) Filter     ║
║   CV1     CV2     CV3     CV4                                        ║
╚══════════════════════════════════════════════════════════════════════╝

                  ← CV1  ← CV2  ← CV3  ← CV4     (jacks TRS 3.5mm)
                  ← MIDI IN    ← MIDI OUT    ← USB MIDI
                  ← Phones L/R         ← Line Out L/R
```

---

## 2. PRIMEROS PASOS

### Encendido

1. Conecta el cable USB-C de alimentación al conector USB-C del panel trasero
2. El display muestra el splash screen "DigitalSynth v1.0" durante 1 segundo
3. Los 16 step buttons hacen un sweep de luz blanca de izquierda a derecha
4. El secuenciador arranca en estado **STOPPED** con el **Track 1** seleccionado y la **página TRACK** activa

### Primer sonido en 60 segundos

1. Conecta un sintetizador o módulo Eurorack a **CV1** (jack TRS 3.5mm)
2. Presiona **SW1**, **SW3**, **SW5** y **SW9** — se encienden en rojo (steps activos en Track 1)
3. Presiona **PLAY ▶** — el cursor blanco comienza a recorrer los 16 pasos
4. Gira **ENC1** para cambiar la nota base del track — escucha cómo cambia el pitch
5. Presiona **STOP ■** para detener. Presiona **STOP ■** de nuevo para volver al paso 1

---

## 3. CONCEPTOS FUNDAMENTALES

### Tracks

El DigitalSynth tiene **4 tracks independientes**, uno por cada salida CV.
Piensa en cada track como un secuenciador autónomo que controla un instrumento diferente:
- **Track 1 / CV1** → Melodía principal (synth lead)
- **Track 2 / CV2** → Bajo
- **Track 3 / CV3** → Acorde o pad
- **Track 4 / CV4** → Percusión o otro elemento

Cada track tiene su propio patrón de 16 pasos, su propio tempo relativo, sus propios LFOs y sus propios parámetros de voz.

### El track activo

Solo **un track está activo para edición** a la vez. Los 4 tracks **siempre se reproducen simultáneamente** cuando el secuenciador está corriendo, pero los encoders y los step buttons siempre afectan al track activo.

> **Seleccionar un track no lo silencia.** Solo cambia qué track estás editando.

### Steps

Cada track tiene **16 pasos (steps)**. Cada step puede estar:
- **Vacío** — el secuenciador pasa por él sin generar gate ni CV
- **Activo** — genera un pulso de gate y un voltaje CV correspondiente a la nota programada

### Páginas

Los 8 encoders tienen **funciones diferentes** según qué página está activa:

| Página | Los encoders controlan |
|---|---|
| **TRACK** | Parámetros de voz: nota, octava, portamento, gate, etc. |
| **SEQ** | Parámetros del secuenciador: longitud, dirección, swing, tempo |
| **MOD** | Dos LFOs: rate, forma de onda, depth, destino |
| **SONG** | Gestión de patrones, bancos, proyecto |

---

## 4. TRACKS Y SALIDAS CV

### Seleccionar un track

Presiona **T1**, **T2**, **T3** o **T4**.

El botón del track seleccionado se **enciende** . Los demás se apagan.
Los step buttons inmediatamente muestran en color el patrón de ese track.

### Mute / Unmute un track

**Hold** el botón de track por **500ms**.

El LED del track empieza a **parpadear lentamente** — el track está muteado.
El secuenciador sigue corriendo internamente. Al desactivar el mute, el track se reincorpora en sincronía.

### Solo — escuchar solo un track

**SHIFT + T1/T2/T3/T4** — todos los demás tracks se mutean instantáneamente.

Para volver a escuchar todos: **SHIFT + T1** de nuevo (toggle), o presiona los tracks muteados individualmente.

### Voltaje de salida CV

Por defecto, el rango de salida es **0–5V**, que corresponde a **5 octavas** en un sintetizador con 1V/oct estándar.

Puedes cambiar el rango en la página TRACK con **ENC4**:
- `2.5V` → 2.5 octavas (mayor precisión, menor rango)
- `5.0V` → 5 octavas (configuración recomendada)

> Asegúrate de que tu sintetizador destino esté calibrado en **1V/oct** y que el cable sea TRS o jack mono.

---

## 5. PROGRAMAR UNA SECUENCIA

### Método básico — Tap programming

1. Selecciona el track (T1–T4)
2. Presiona los **step buttons** (SW1–SW16) para activar/desactivar pasos
   - LED encendido = paso activo
   - LED apagado = paso vacío
3. Presiona **PLAY ▶** para escuchar el resultado
4. Modifica steps mientras el secuenciador corre — los cambios son auditivos inmediatamente

### Indicadores visuales durante la reproducción

Mientras el secuenciador corre, el **cursor blanco** recorre los step buttons indicando qué paso se está ejecutando. Los steps activos muestran el color del track.

### Cambiar la nota de un step

1. **Hold** el step button que quieras editar (mantenlo presionado)
2. Gira **ENC1** — la nota cambia en semitonos mientras el step está held
3. Gira **ENC2** — cambia la octava del step
4. Suelta el step — el cambio queda guardado

El display muestra los parámetros del step mientras lo tienes presionado.

### Programar en tiempo real (Live Record)

1. Presiona **REC ●** — el LED de REC se enciende y el borde de los step buttons parpadea en rojo
2. Presiona **PLAY ▶** si el secuenciador no está corriendo
3. Presiona los step buttons al ritmo que quieras — cada pulsación activa ese step en la posición del cursor en tiempo real
4. Presiona **REC ●** de nuevo para salir del modo grabación (el secuenciador sigue corriendo)

---

## 6. EDITAR PARÁMETROS DE UN STEP

Cada step tiene sus propios parámetros que puedes editar de forma independiente.

### Entrar al modo step edit

**Hold** (mantén presionado) cualquier step button activo.

Mientras lo mantienes:
- El step se resalta en **magenta**
- El display muestra los 8 parámetros del step
- Los encoders controlan esos parámetros

### Parámetros editables por step

| Encoder | Parámetro | Descripción |
|---|---|---|
| ENC1 | Nota | Semitono, relativo a la nota base del track |
| ENC2 | Octava | Octava del step (±3 octavas desde la del track) |
| ENC3 | Slide | On/Off — portamento hacia el siguiente step activo |
| ENC4 | — | (reservado) |
| ENC5 | Gate length | Duración del gate, override del valor del track |
| ENC6 | Velocidad | Nivel de voltaje CV (0–100%) |
| ENC7 | Accent | On/Off — envía voltaje extra en este step |
| ENC8 | Probabilidad | % de que este step suene en cada pasada |

### Trig Conditions (condiciones avanzadas)

Con el step held: **SHIFT + ENC1** (gira) para elegir una condición:

| Condición | Significado |
|---|---|
| **Free** | Siempre suena (comportamiento por defecto) |
| **1st** | Solo suena la primera vez que pasa el loop |
| **Fill** | Solo suena cuando FILL está activo |
| **NotFill** | Suena siempre excepto cuando FILL está activo |
| **Pre** | Pre-trig: prepara el gate medio step antes (legato suave) |
| **1:2** | Suena cada 2 veces que pasa el loop |
| **1:3** | Suena cada 3 veces |
| **2:3** | Suena 2 de cada 3 veces |

> Los steps con probabilidad < 100% o con condición tienen un LED que parpadea levemente para indicarlo.

---

## 7. PÁGINA TRACK — VOZ Y CV

Presiona **TRACK** para activar esta página.

Los encoders controlan los parámetros de voz del track activo.

| Encoder | Parámetro | Rango | Notas |
|---|---|---|---|
| **ENC1** | Nota base | C0 – B7 | Offset de pitch de toda la secuencia. Gira para transponer |
| **ENC2** | Octava | 0 – 7 | Transpone toda la secuencia en octavas |
| **ENC3** | Portamento | Off / 1ms–1000ms | Push ENC3 = activa/desactiva. Gira para ajustar el tiempo |
| **ENC4** | Rango CV | 2.5V / 5V | Push para togglear. Afecta el voltaje máximo de salida |
| **ENC5** | Gate length | 1% – 100% | Longitud del gate como porcentaje del tiempo del step |
| **ENC6** | Nivel CV | 0% – 100% | Atenúa la salida CV del track (volumen del pitch) |
| **ENC7** | Accent | 0% – 100% | Voltaje extra en los steps marcados como accent |
| **ENC8** | Bend range | 0 – 24 semitonos | Rango del pitch bend (si se implementa joystick o LFO) |

**Push de encoder en página TRACK:**
- Cualquier push: resetea ese parámetro a su valor por defecto

---

## 8. PÁGINA SEQ — SECUENCIADOR

Presiona **SEQ** para activar esta página.

| Encoder | Parámetro | Rango | Notas |
|---|---|---|---|
| **ENC1** | Longitud del patrón | 1 – 16 steps | El loop termina en este step y vuelve al 1 |
| **ENC2** | Escala musical | Cromática / Mayor / Menor / Pentatónica / Dórica... | Cuantiza las notas de los steps a la escala |
| **ENC3** | Nota raíz de escala | C – B | Tónica de la escala. Funciona junto a ENC2 |
| **ENC4** | Dirección | → / ← / ↔ / Random / Pendulum | Cómo recorre los steps el secuenciador |
| **ENC5** | Tempo (BPM) | 30 – 300 BPM | Tempo global del proyecto. Igual que en SONG |
| **ENC6** | Swing | 50% – 75% | Retrasa los steps pares para dar feeling rítmico |
| **ENC7** | Multiplicador | ÷4 / ÷2 / ×1 / ×2 / ×4 | Velocidad del track respecto al tempo master |
| **ENC8** | Probabilidad global | 0% – 100% | Probabilidad de que cada step del track suene |

### Modos de dirección

| Modo | Descripción | Ejemplo con 4 steps activos |
|---|---|---|
| **→ Forward** | Reproducción normal de izquierda a derecha | 1→2→3→4→1→2... |
| **← Reverse** | Reproducción de derecha a izquierda | 4→3→2→1→4→3... |
| **↔ Ping-pong** | Va y vuelve sin repetir los extremos | 1→2→3→4→3→2→1→2... |
| **Random** | Cada step elige un paso al azar | 3→1→4→1→2→3... |
| **Pendulum** | Va y vuelve repitiendo los extremos | 1→2→3→4→4→3→2→1→1... |

### Multiplicador de tempo

El multiplicador cambia la **velocidad relativa** de ese track.

| Valor | Efecto | Uso típico |
|---|---|---|
| ÷4 | Cuatro veces más lento | Pads muy lentos, drones |
| ÷2 | Dos veces más lento | Melodías lentas sobre ritmo rápido |
| ×1 | Tempo normal | Uso general |
| ×2 | Doble de rápido | Arpegios, hi-hats |
| ×4 | Cuádruple de rápido | Tremolos, texturas |

---

## 9. PÁGINA MOD — MODULACIÓN LFO

Presiona **MOD** para activar esta página.

Cada track tiene **2 LFOs independientes**. Los encoders 1–4 controlan el LFO1 y los encoders 5–8 el LFO2.

| Encoder | LFO | Parámetro | Rango |
|---|---|---|---|
| **ENC1** | LFO1 | Rate (velocidad) | 0.01 – 500 Hz / Sync: ÷16 a ×4 |
| **ENC2** | LFO1 | Shape (forma de onda) | Sine / Triangle / Saw / Ramp / Square / S&H |
| **ENC3** | LFO1 | Depth (profundidad) | -100% a +100% |
| **ENC4** | LFO1 | Destino | Pitch / CV Level / Gate Length / Accent |
| **ENC5** | LFO2 | Rate | 0.01 – 500 Hz / Sync |
| **ENC6** | LFO2 | Shape | Sine / Triangle / Saw / Ramp / Square / S&H |
| **ENC7** | LFO2 | Depth | -100% a +100% |
| **ENC8** | LFO2 | Destino | Pitch / CV Level / Gate Length / Accent |

**Push ENC1 / Push ENC5:** Activa/desactiva la sincronización del LFO al tempo del secuenciador.

### Formas de onda del LFO

| Forma | Descripción | Uso típico |
|---|---|---|
| **Sine** | Onda sinusoidal suave | Vibrato, tremolo natural |
| **Triangle** | Onda triangular lineal | Vibrato más percibido |
| **Saw** | Diente de sierra descendente | Filtro decayendo |
| **Ramp** | Diente de sierra ascendente | Portamento manual, glide |
| **Square** | Onda cuadrada | Trémolo, intervalos de octava |
| **S&H** | Sample & Hold — valor aleatorio en cada step | Secuencias aleatorias sincronizadas |

### Destinos del LFO

| Destino | Efecto |
|---|---|
| **Pitch** | Modula la nota de salida — vibrato, glide, pitch sweep |
| **CV Level** | Modula el nivel de voltaje — tremolo, swell |
| **Gate Length** | Modula la duración de los gates — variaciones rítmicas |
| **Accent** | Modula el accent — acentuaciones dinámicas variables |

---

## 10. PÁGINA SONG — PATRONES Y PROYECTO

Presiona **SONG** para activar esta página.

| Encoder | Parámetro | Descripción |
|---|---|---|
| **ENC1** | Patrón actual | Selecciona el patrón 1–128 a editar o reproducir |
| **ENC2** | Modo de loop | Loop / One-shot / Ping-pong (del patrón completo) |
| **ENC3** | Bank | A / B / C / D (cada banco contiene 32 patrones) |
| **ENC4** | Patrón destino | Para operaciones COPY, PASTE y MOVE |
| **ENC5** | Tempo master | BPM global del proyecto (idéntico al de página SEQ) |
| **ENC6** | Volumen master | Atenúa globalmente todos los CV de salida |
| **ENC7** | Song chain index | Posición en la lista de encadenado (song mode) |
| **ENC8** | Slot de guardado | Ranura 1–8 para guardar el proyecto |

**Push ENC8** (hold 1 segundo): Guarda el proyecto en el slot seleccionado a la memoria Flash.

### Estructura de bancos y patrones

```
Bank A → Patrones 1–32    (ej: introducción, verso)
Bank B → Patrones 33–64   (ej: coro, bridge)
Bank C → Patrones 65–96   (ej: variaciones, drops)
Bank D → Patrones 97–128  (ej: fills, transiciones, solos)
```

---

## 11. TRANSPORTE Y GRABACIÓN

### PLAY ▶

Inicia la reproducción desde la posición actual del cursor.
Si el secuenciador ya está corriendo, no hace nada.

### STOP ■

- **Primera pulsación:** Para el secuenciador. Los CV quedan en el último valor.
- **Segunda pulsación:** Vuelve al step 1 de todos los tracks.

### REC ●

Activa el modo de grabación en tiempo real.

En modo REC + PLAYING:
- Presionar un step button lo activa en la posición **actual** del cursor, no en la posición física del botón
- Los parámetros de track se aplican al step grabado en ese momento
- El LED de REC parpadea para indicar que estás grabando

### TAP

Ajusta el tempo golpeando el botón al ritmo deseado.
El sistema promedia las últimas **4 pulsaciones** para calcular el BPM.
El BPM resultante se muestra en el display.

### Funciones con SHIFT

| Combinación | Función |
|---|---|
| **SHIFT + PLAY** | Restart — vuelve al step 1 de todos los tracks y arranca de inmediato |
| **SHIFT + STOP** | **PANIC** — todos los CV van a 0V instantáneamente. Útil si un synth queda triggerado |
| **SHIFT + REC** | Overdub — graba encima sin borrar los steps existentes |
| **SHIFT + TAP** | Abre pantalla de entrada de BPM numérico. Gira ENC5 para ajustar |

---

## 12. GESTIÓN DE PATRONES

### Cambiar de patrón

**PATT ◄** / **PATT ►** — Avanza o retrocede al patrón anterior/siguiente.

El cambio se produce al **final del patrón actual** (cuantizado), no de forma inmediata. Esto permite preparar el cambio antes de que ocurra, igual que en Elektron.

> Para cambio inmediato: **SHIFT + PATT ◄/►**

### Bancos de patrones

**SHIFT + PATT ◄** — Retrocede un banco completo (A→D→C→B→A).
**SHIFT + PATT ►** — Avanza un banco completo (A→B→C→D→A).

### Copiar, pegar y borrar patrones

| Operación | Acción | Notas |
|---|---|---|
| **COPY** | Copia el track activo del patrón actual al buffer | |
| **PASTE** | Pega el buffer en el track activo del patrón actual | El buffer se mantiene hasta el siguiente COPY |
| **CLEAR** | **Hold 1 segundo** — borra el track activo | El hold evita borrados accidentales |
| **SHIFT + COPY** | Copia los 4 tracks completos del patrón | Copia global |
| **SHIFT + PASTE** | Pregunta en el display qué tracks pegar | Paste selectivo |
| **SHIFT + CLEAR** | **Hold 1 segundo** — borra el patrón completo (4 tracks) | |

---

## 13. SONG MODE — ENCADENADO DE PATRONES

El Song Mode permite reproducir una lista de patrones en orden, creando una canción completa.

### Activar Song Mode

Presiona **CHAIN** — el botón se enciende indicando que Song Mode está activo.

### Editar la lista de song

**SHIFT + CHAIN** — Entra en el editor de song chain.

En el editor:
- **ENC7** (en página SONG) navega por las entradas de la lista
- Cada entrada especifica: **Patrón** + **Repeticiones** + **Track mutes** de esa sección
- Los step buttons se pueden usar para activar/desactivar mutes por track en esa entrada

Ejemplo de song chain completa:
```
Pos 1: Patrón 1  × 4 reps  [T1:on  T2:off T3:off T4:off]  ← solo lead
Pos 2: Patrón 1  × 4 reps  [T1:on  T2:on  T3:off T4:off]  ← + bajo
Pos 3: Patrón 5  × 8 reps  [T1:on  T2:on  T3:on  T4:on ]  ← full
Pos 4: Patrón 9  × 4 reps  [T1:on  T2:on  T3:on  T4:on ]  ← coro
Pos 5: Patrón 1  × 2 reps  [T1:on  T2:off T3:off T4:off]  ← outro
```

### Desactivar Song Mode

Presiona **CHAIN** de nuevo — vuelve al modo de patrón único en loop.

---

## 14. FILL — VARIACIONES EN VIVO

El Fill permite activar variaciones del patrón en tiempo real, ideal para transiciones y momentos de tensión.

### Activar Fill

**Hold FILL** — mientras mantienes el botón presionado, el secuenciador usa el "patrón fill".

Los steps con condición `Fill` suenan, los steps con condición `NotFill` se silencian.
Al soltar el botón, vuelve al comportamiento normal.

> El Fill es sensible al tempo — siempre termina en un punto musical limpio.

### Fill permanente (modo jam)

**SHIFT + FILL** — El fill queda activo hasta que presiones **SHIFT + FILL** de nuevo.

Útil para improvisar secciones de alta energía sin tener que mantener el dedo.

---

## 15. SISTEMA SHIFT — FUNCIONES AVANZADAS

**SHIFT** es el botón modificador global del DigitalSynth. Mantenerlo presionado mientras usas otros controles activa funciones alternativas.

### Resumen de combinaciones SHIFT

#### Transporte y sistema

| Combinación | Función |
|---|---|
| SHIFT + PLAY | Restart completo (step 1, todos los tracks) |
| SHIFT + STOP | PANIC — CV todos a 0V |
| SHIFT + REC | Overdub mode |
| SHIFT + TAP | Entrada numérica de BPM |

#### Tracks

| Combinación | Función |
|---|---|
| SHIFT + T1 | Solo Track 1 (mutea T2, T3, T4) |
| SHIFT + T2 | Solo Track 2 |
| SHIFT + T3 | Solo Track 3 |
| SHIFT + T4 | Solo Track 4 |
| SHIFT + T1+T2 | Solo Tracks 1 y 2 juntos |

#### Páginas (sub-páginas avanzadas)

| Combinación | Función |
|---|---|
| SHIFT + TRACK | Sub-página: CV routing y calibración |
| SHIFT + SEQ | Sub-página: escala global del proyecto |
| SHIFT + MOD | Sub-página: envelope (futuro) |
| SHIFT + SONG | Sub-página: configuración del proyecto |

#### Patrones

| Combinación | Función |
|---|---|
| SHIFT + PATT ◄ | Retrocede un banco entero |
| SHIFT + PATT ► | Avanza un banco entero |
| SHIFT + CHAIN | Editor de song chain |
| SHIFT + FILL | Fill permanente (toggle) |

#### Edición

| Combinación | Función |
|---|---|
| SHIFT + COPY | Copia el patrón completo (4 tracks) |
| SHIFT + PASTE | Paste selectivo con confirmación |
| SHIFT + CLEAR (hold) | Borra el patrón completo |
| SHIFT + step | Toggle accent en ese step |
| SHIFT + hold step + ENC1 | Condición del step (Free/1st/Fill/etc.) |
| SHIFT + hold step + ENC8 | Ratio de condición (1:2, 1:3...) |

#### Encoders

| Combinación | Función |
|---|---|
| SHIFT + ENC8 push | Undo — deshace el último cambio de parámetro |
| SHIFT + ENC1 push | Randomize parámetro 1 del track activo |

---

## 16. TRIG CONDITIONS — PROBABILIDAD Y CONDICIONES

Las Trig Conditions permiten que los steps se comporten de forma diferente en cada pasada del loop, creando variaciones automáticas y patrones evolutivos.

### Acceder a las condiciones

**Hold el step + SHIFT + girar ENC1** → elige la condición.

El display muestra la condición asignada mientras el step está held.

### Condiciones disponibles

| Condición | Comportamiento |
|---|---|
| **Free** | Siempre suena. Comportamiento por defecto |
| **1st** | Solo suena en la **primera** repetición del patrón. Perfecto para notas de introducción |
| **Fill** | Solo suena cuando el botón FILL está activo. Para variaciones de relleno |
| **NotFill** | Suena siempre **excepto** cuando FILL está activo. Para quitar elementos durante el fill |
| **Pre** | Pre-trig: el gate empieza medio step antes. Crea legatos y slides naturales |
| **1:2** | Suena una de cada 2 repeticiones del patrón |
| **1:3** | Suena una de cada 3 |
| **1:4** | Suena una de cada 4 |
| **2:3** | Suena 2 de cada 3 repeticiones |
| **3:4** | Suena 3 de cada 4 repeticiones |

### Probabilidad por step

**Hold el step + girar ENC8** → probabilidad del step (0–100%).

A 100% el step siempre suena. A 50% suena en la mitad de las veces (aleatorio en cada pasada). A 0% nunca suena (equivalente a desactivar el step pero conservando los parámetros).

> **Consejo:** Combina condición `Free` con probabilidad 70–90% en algunos steps de melodía para crear secuencias que nunca suenan exactamente igual dos veces sin perder la estructura base.

---

## 17. GUARDAR Y CARGAR PROYECTOS

El DigitalSynth puede almacenar hasta **8 proyectos completos** en la memoria Flash interna (W25Q128, 16MB).

Un proyecto incluye: todos los patrones (1–128), todos los parámetros de voz de los 4 tracks, la song chain y la configuración global.

### Guardar un proyecto

**Método 1 (rápido):**
1. Ve a página **SONG**
2. Gira **ENC8** para seleccionar el slot de guardado (1–8)
3. **Hold ENC8** durante 1 segundo
4. Los step buttons parpadean verde 2 veces — guardado exitoso

**Método 2 (con confirmación):**
1. **SHIFT + PLAY** (hold 2 segundos) desde cualquier página
2. El display pregunta: `SAVE TO SLOT [1-8]?`
3. Gira ENC8 para elegir slot, press ENC8 para confirmar

### Cargar un proyecto

1. Ve a página **SONG**
2. Gira **ENC8** para seleccionar el slot
3. Hold **ENC1** durante 1 segundo (botón de carga)
4. Los step buttons parpadean azul 2 veces — cargado exitoso

> **Aviso:** Cargar sobreescribe el proyecto en memoria RAM. Si tienes cambios sin guardar, guarda antes.

### Auto-save

El DigitalSynth **no tiene auto-save** automático. Los cambios en RAM se pierden si se apaga sin guardar.

**Buena práctica:** Guarda cada vez que termines de programar algo que quieras conservar.

---

## 18. REFERENCIA RÁPIDA DE BOTONES

### Botones de Track (T1–T4)

| Acción | Resultado |
|---|---|
| Press | Selecciona el track para edición |
| Hold 500ms | Mute / Unmute el track |
| SHIFT + Press | Solo ese track (mutea los demás) |

### Step Buttons (SW1–SW16)

| Acción | Resultado |
|---|---|
| Press breve | Activa / Desactiva el step |
| Hold | Step edit — encoders controlan parámetros del step |
| Hold + ENC1 | Nota del step (semitono) |
| Hold + ENC2 | Octava del step |
| Hold + ENC5 | Gate length del step |
| Hold + ENC8 | Probabilidad del step |
| SHIFT + Hold + ENC1 | Condición del step (Free/1st/Fill...) |
| SHIFT + Press | Toggle accent en el step |

### Encoders (ENC1–ENC8)

| Acción | Resultado |
|---|---|
| Girar | Ajusta el parámetro de la página activa |
| Push breve | Resetea el parámetro a su valor por defecto |
| Push hold | Función especial (ver sección de página correspondiente) |
| SHIFT + Push ENC8 | Undo — deshace el último cambio |

### Tabla de botones de función completa

| Botón | Normal | SHIFT + | Hold |
|---|---|---|---|
| TRACK | Página TRACK | Sub-página avanzada | — |
| SEQ | Página SEQ | Escala global | — |
| MOD | Página MOD | Envelope (futuro) | — |
| SONG | Página SONG | Config del proyecto | — |
| PLAY ▶ | Start/Continue | Restart completo | Guardar rápido (2s) |
| STOP ■ | Stop / Step 1 | PANIC (CV→0V) | — |
| REC ● | Rec on/off | Overdub | — |
| TAP | Tap tempo | BPM numérico | — |
| PATT ◄ | Patrón anterior | Banco anterior | — |
| PATT ► | Patrón siguiente | Banco siguiente | — |
| CHAIN | Song mode on/off | Editor song chain | — |
| FILL | Fill mientras held | Fill permanente | — |
| SHIFT | Modificador | — | — |
| COPY | Copia track activo | Copia patrón completo | — |
| PASTE | Pega en track activo | Paste selectivo | — |
| CLEAR | — | — | Borra track (1s) / patrón completo con SHIFT (1s) |

---

## APÉNDICE A — GLOSARIO

| Término | Significado |
|---|---|
| **CV** | Control Voltage — voltaje analógico (0–5V) que controla el pitch de un sintetizador |
| **Gate** | Señal digital ON/OFF que indica cuándo una nota está activa (≈ +5V = ON, 0V = OFF) |
| **1V/oct** | Estándar de control en el que cada octava equivale a 1V de diferencia en CV |
| **Step** | Cada uno de los 16 pasos de la secuencia |
| **Pattern / Patrón** | Un loop completo de los 4 tracks (hasta 128 guardables) |
| **Track** | Uno de los 4 canales independientes del secuenciador (= una salida CV) |
| **LFO** | Low Frequency Oscillator — oscilador lento que modula parámetros |
| **S&H** | Sample & Hold — el LFO toma un valor aleatorio en cada step |
| **BPM** | Beats Per Minute — velocidad del tempo |
| **Swing** | Retraso de los steps pares para crear sensación rítmica no quantizada |
| **Trig Condition** | Condición que determina cuándo un step suena (siempre, solo al principio, con fill, etc.) |
| **Song Chain** | Lista ordenada de patrones para reproducción de una canción completa |
| **Fill** | Variación temporal del patrón, activada manualmente |
| **Accent** | Nivel de voltaje extra en un step para dar énfasis dinámico |
| **Slide / Portamento** | Transición suave de pitch entre un step y el siguiente |

---

## APÉNDICE B — ESPECIFICACIONES TÉCNICAS

| Característica | Valor |
|---|---|
| Salidas CV | 4 × TRS 3.5mm, 0–5V (ajustable), impedancia de salida 100Ω |
| Resolución CV | 16-bit (DAC8565), ~76µV por paso |
| Gate output | Via CV implícito — 0V = off, valor programado = on |
| Salida de audio | 2 × RCA (L/R) + 3.5mm stereo, PCM5122 32-bit/384kHz |
| USB MIDI | Class-compliant, USB-B, no requiere drivers |
| MIDI DIN | IN + OUT, 31.25 kbaud estándar |
| Almacenamiento | 16MB Flash SPI (W25Q128) — hasta 8 proyectos con 128 patrones cada uno |
| Alimentación | USB-C 5V / 1A |
| Procesador | STM32H743, 480MHz, ARM Cortex-M7 |
| Tempo | 30–300 BPM, resolución 0.1 BPM |
| Tracks | 4 independientes |
| Pasos por track | 1–16 (ajustable) |
| Patrones | 128 por proyecto (4 bancos × 32) |
| Proyectos | 8 slots en Flash |

---

## HISTORIAL DE CAMBIOS DEL MANUAL

| Versión | Fecha | Descripción |
|---|---|---|
| 1.0 | 25 Feb 2026 | Versión inicial — workflow completo documentado |
