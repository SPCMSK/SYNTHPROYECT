# Plan de Implementación por Etapas
## DigitalSynth v1.0 — Réplica funcional Elektron Digitone (100% digital)

> **Metodología:** Desarrollo incremental con validación de audio al final de cada fase.
> Nunca se avanza a la siguiente etapa hasta que la actual produce audio correcto y medible.

---

## Resumen Visual de Fases

```
FASE 1 ──► FASE 2 ──► FASE 3 ──► FASE 4 ──► FASE 5 ──► FASE 6
KiCad PCB  MCU+DAC    Motor FM   USB MIDI   Panel+UI   Chasis
 STM32H7   Digital    DSP Voces   + Presets  Digitone   Final
  4 sem      4 sem      4 sem       3 sem      3 sem     4 sem
```

**Duración total estimada:** 22 semanas (~5.5 meses con dedicación parcial)

> **Cambio v2.0:** Se eliminó FASE 3 (cadena analógica AS3320+THAT2180) y se reemplazó
> por la FASE 3 de Motor FM + filtro SVF digital. El proyecto es ahora 100% digital.

---

## FASE 1 — Diseño Esquemático + PCB en KiCad (STM32H7 directo)
**Duración:** 4 semanas
**Objetivo:** Diseñar y enviar a fabricar el PCB principal con STM32H743 — sin prototipo Teensy.

> **Decisión de diseño:** Se eliminó la fase Teensy. El firmware clave (`adsr_engine.c`,
> `usb_midi_desc.c`) ya está desarrollado. Se va directamente al PCB final en STM32H743.

### Semana 1 — Esquemático: Alimentación + MCU
- [ ] Instalar KiCad 8, configurar librerías, crear proyecto DigitalSynth
- [ ] Diseñar bloque de alimentación: USB-C 5V → LT3042 (+3.3V_ANA para PCM5122) + ADP3335 (+3.3V_DIG)
- [ ] Resistencias 5.1kΩ en CC1/CC2 para señalar 1.5A al cargador USB-C
- [ ] **Sin LT3094, sin LT1054** — no hay rail negativo (eliminado junto con la cadena analógica)
- [ ] Diseñar bloque STM32H743: decoupling pin a pin, cristal 25MHz, VCAP1/VCAP2, SWD
- [ ] ERC del bloque power + MCU — 0 errores antes de continuar

### Semana 2 — Esquemático: Audio e Interfaz
- [ ] Diseñar USB-B (MIDI) con USBLC6-2 + polyfuse
- [ ] Diseñar PCM5122PW (I2S 32-bit, I2C address 0x4C): AVDD/CPVDD → +3.3V_ANA, DVDD → +3.3V_DIG
  - XSMT (pin 25) → GPIO PC3 con pull-up 10kΩ (soft mute para evitar pop de encendido)
  - Output filter: 470Ω + 2.2nF → bifurcación RCA + Jack 6.35mm con 100Ω de aislamiento
- [ ] Diseñar DAC8565IAPWR (SPI, 4-ch CV out): AVDD → +5V VBUS via ferrite, IOVDD → +3.3V_DIG
  - 4 salidas a jacks TRS 3.5mm (CV out para Eurorack y equipos externos)
- [ ] Diseñar bus I2C: PCM5122 (0x4C) + SSD1306 (0x3C) + PCA9685 (0x40) + MCP23017 (0x20)
- [ ] Diseñar 8× encoders EC11 (pull-ups + anti-rebote) + 2× pots (Cutoff + Volume)
- [ ] ERC final completo — 0 errores — **esquemático listo**

### Semana 3 — Layout PCB Principal (100×100mm)
- [ ] Asignar footprints, importar al PCB editor, organizar bloques funcionales
- [ ] Planos DGND/AGND separados, unión star ground única, net classes de potencia
- [ ] Colocar ICs: STM32 primero, cristal <5mm, decoupling <0.5mm de cada VDD
- [ ] Trazar potencia → rutas analógicas → rutas digitales (SPI, I2S, I2C, USB 90Ω)
- [ ] DRC sin errores, via stitching AGND, thermal vias LT3042/LT3094, 3D check

### Semana 4 — Envío fabricación + Firmware base
- [ ] Exportar Gerbers + BOM + CPL — **ENVIAR PEDIDO JLCPCB**
- [ ] STM32CubeMX: clock tree (480MHz PLL1, 48MHz PLL3), SAI1, SPI1, SPI2, USB, I2C1, USART1
- [ ] Portar `adsr_engine.c` + `usb_midi_desc.c` — compilar sin warnings en CubeIDE
- [ ] Implementar DMA doble buffer SAI1 (AXI SRAM, no DTCM)
- [ ] Driver PCA9685: init + `PCA9685_SetPWM(ch, on, off)` por I2C
- [ ] Driver MCP23017: init + lectura de 16 pines por interrupción INT + debounce

### Entregables al finalizar
- Gerbers enviados a JLCPCB (5 unidades Rev.A)
- Firmware base compilando: SAI1 generando silencio (sin clicks), USB MIDI enumera
- PCB panel (Panel PCB diseñado en paralelo, conectado por JST ribbon)

---

## FASE 2 — Bring-Up PCB y Validación Digital
**Duración:** 4 semanas
**Objetivo:** Encender el PCB Rev.A por primera vez, validar audio digital I2S, USB MIDI y CV.

### Semana 5 — Recepción de PCBs y pre-test
- [ ] Inspección visual con lupa: soldaduras SMD, orientación ICs, cortocircuitos
- [ ] Medir +5V_USB, +3V3_DIG, +5V_ANA, -5V_ANA con multímetro antes de conectar ICs
- [ ] Alimentar con fuente laboratorio limitada a 100mA — verificar consumo < 80mA en frío
- [ ] Programar STM32H7 via SWD con programa de test ("blink LED")
- [ ] Verificar clock tree con osciloscopio: 480MHz HCLK confirmado via MCO1

### Semana 6 — Validación I2S y USB MIDI
- [ ] Verificar señales SAI1 con osciloscopio: BCK, LRCK, MCLK, DATA correctos
- [ ] PCM5122 genera tono de 1kHz en salida de audio — medir con osciloscopio + auriculares
- [ ] USB MIDI enumera correctamente en Windows/Linux (Product="DigitalSynth MIDI")
- [ ] Enviar NoteOn desde Ableton, verificar recepción en STM32 con SWD debugger
- [ ] DAC8565 produce voltajes CV medibles con multímetro (0.0V, 1.25V, 2.5V, 5.0V)

### Semana 7 — Validación I2C, Flash y UI básica
- [ ] I2C bus: leer registros de PCM5242, SSD1306 muestra un patrón estático
- [ ] PCA9685: 16× LEDs encendidos y apagados secuencialmente
- [ ] MCP23017: press de cada switch detectado en ISR con INT_A
- [ ] W25Q128: write/read 256 bytes — verificar integridad
- [ ] 8× encoders: giro derecha/izquierda + push detectados correctamente

### Semana 8 — Integración firmware base
- [ ] Motor ADSR genera envolventes: Attack/Decay/Sustain/Release audibles en salida PCM5242
- [ ] Control de ADSR por MIDI CC (CC73=Attack, CC72=Release, CC75=Decay)
- [ ] OLED muestra valores de parámetros al girar encoder
- [ ] MCP23017 + 16 switches controlan 16 steps del secuenciador básico
- [ ] **GATE DE SALIDA:** Audio FM con ADSR funcional por USB MIDI

### Entregables al finalizar
- PCB encendido sin cortocircuitos ni componentes dañados
- Audio FM funcional desde Ableton via USB MIDI
- DAC8565 produciendo 4 voltajes CV correctos

### Entregables al finalizar
- Primer PCB funcional con sonido FM por I2S
- DAC8565 enviando voltajes CV medibles con multímetro
- USB MIDI reconocido por Ableton en la nueva placa

---

## FASE 3 — Motor FM + Filtro SVF Digital + Efectos
**Duración:** 4 semanas
**Objetivo:** 4 voces FM completas con filtro por voz, ADSR multi-destino y efectos de master bus.

### Semana 9 — Motor FM 4 operadores
- [ ] Implementar oscilador FM con tabla de senos (1024 muestras, interpolación lineal)
- [ ] 4 operadores por voz: ratio X/Y (flotante), level, feedback de op1→op1
- [ ] 8 algoritmos: implementar código genérico con matriz de conexión carrier/modulator
- [ ] Test: voz single NoteOn MIDI → tono FM audible por SAI1 → PCM5122
- [ ] Comparar formas de onda vs output de sintetizador FM software de referencia

### Semana 10 — Filtro SVF + VCA digital por voz
- [ ] Implementar filtro Halófan-Chamberlin SVF (LP/HP/BP) por voz
- [ ] Coeficientes calculados en tiempo real: `f = 2*sinf(PI*cutoff/48000)`
- [ ] Lógica de saturación suave en lazo de resonancia: `tanhf(band * drive)`
- [ ] VCA: multiplicación de muestra por `env_amp.current_level` (float 0–1)
- [ ] ADSR en 3 instancias por voz (pitch, filtro cutoff, amplitud)
- [ ] Test: sweep de cutoff audible, resonancia estable hasta auto-oscilación

### Semana 11 — Polifonía de 4 voces + asignador
- [ ] Voice allocator: estrategia "robo de voz" (oldest note stolen) para 4 voces
- [ ] Mix estéreo: 4 voces sumadas con pan independiente por voz → buffer SAI1 DMA
- [ ] DC-blocking por voz: `y = 0.9997*(y + x - x_prev)` para eliminar DC del FM
- [ ] Test: acordes de 4 notas simultáneas sin clipping ni glitches

### Semana 12 — Efectos master bus
- [ ] Delay estéreo: buffer circular en AXI SRAM, sync BPM MIDI clock
- [ ] Reverb FDN-8: 8 líneas de delay con matriz de mezcla Hadamard
- [ ] Chorus/flanger: LFO sobre delay corto (1–20ms)
- [ ] Bus insert: dry/wet controlado por encoder / MIDI CC
- [ ] **GATE DE SALIDA:** 4 voces FM con filtro, ADSR y efectos funcionando simultáneamente

### Entregables al finalizar
- 4 voces FM completas funcionando en tiempo real
- Filtro SVF por voz con cutoff + resonancia modulables
- Delay + Reverb en master bus
- Carga de CPU documentada (debe ser < 10% del STM32H743)

---

## FASE 4 — USB MIDI, Presets y Secuenciador
**Duración:** 3 semanas
**Objetivo:** Control completo desde Ableton, presets guardables, secuenciador de 16 pasos.

### Semana 13
- [ ] Portar `usb_midi_driver.c` completo al STM32H7
- [ ] Implementar todos los callbacks MIDI CC (CC74=Cutoff, CC71=Resonance, etc.)
- [ ] Test exhaustivo en Ableton: automatización de parámetros en tiempo real
- [ ] Verificar ausencia de zipper noise con movimientos automáticos de CC

### Semana 14
- [ ] Sistema de presets en Flash QSPI (W25Q128):
  - Estructura de preset: `adsr_params_t` + `fm_params_t` + `svf_params_t` + `effects_params_t`
  - Hasta 512 presets en 16MB de flash
  - Función de guardado/carga desde encoders
- [ ] Implementar "Parameter Locks" estilo Digitone:
  - Cada uno de los 16 pasos del secuenciador puede tener valores de parámetro diferentes
  - Interpolación entre pasos para transiciones suaves

### Semana 15
- [ ] Secuenciador de 16 pasos:
  - Sincronización con MIDI Clock del host (Ableton)
  - Modo interno: BPM propio con tap-tempo
  - Modo externo: slaved al MIDI Clock
  - Parameter locks: hasta 8 parámetros por paso

### Entregables al finalizar
- 10 presets de fábrica guardados y funcionando
- Secuenciador sincronizado con Ableton
- Automatización de todos los parámetros desde DAW

---

## FASE 5 — Interfaz de Usuario (Panel Frontal)
**Duración:** 3 semanas
**Objetivo:** El instrumento es operable sin computadora.

### Semana 16
- [ ] Diseño del panel frontal en Inkscape/Illustrator:
  - Tamaño objetivo: 300×180mm (formato de escritorio, Panel PCB separado del Main)
  - Materiales: aluminio 3mm + serigrafía UV o vinilo impreso
  - Distribución inspirada en Digitone: 16 steps | 8 encoders | OLED | función buttons
- [ ] **UI estilo Digitone:** 8× encoders EC11 con push → parámetros en páginas de OLED
- [ ] 2× pots físicos únicamente: Master Volume (RV3) + Filter Cutoff (RV4)
- [ ] 16× switches mecánicos estilo Cherry MX para steps del secuenciador
- [ ] ~12× botones de función (Play/Stop/Record/Page/Bank/etc.) → MCP23017
- [ ] Display OLED 128×64 SSD1306 para valores actuales, nombres de preset, páginas

### Semana 17
- [ ] Firmware de UI (estilo Digitone):
  - Máquina de estados: SYNTH / SEQ / EFFECTS / PRESET por página de OLED
  - 8 encoders asignados dinámicamente según página activa
  - MCP23017 (I2C 0x20): ISR en INT_A → lectura de 16 GPIO (switches + botones)
- [ ] LEDs para step sequencer: 16× LED SMD 0603 controlados por PCA9685 (I2C 0x40)
  - PWM 12-bit: brillo variable (dim=inactivo, full=activo, blink=mute)

### Semana 18
- [ ] Test de usuario con personas sin contexto técnico:
  - ¿Pueden cambiar un preset sin instrucciones?
  - ¿El display es legible?
  - ¿Los encoders tienen buena respuesta táctil?
- [ ] Iteración de UI basada en feedback

### Entregables al finalizar
- Panel frontal funcional completo
- Firmware de UI sin bugs de navegación
- Manual de usuario básico (1 página)

---

## FASE 6 — Integración Final y Chasis
**Duración:** 4 semanas
**Objetivo:** Producto listo para uso en estudio.

### Semana 19–20
- [ ] PCB revisión 2: corregir todos los bugs de la Rev 1
- [ ] Agregar protección de entrada (diodo TVS en todos los jack externos)
- [ ] Alimentación: USB-C 5V (sin IRM-30-12) — confirmar polyfuse 2A y cables USB-C de calidad
- [ ] Test de EMI informal: verificar sin ruido de switching con analizador de espectro de audio

### Semana 21
- [ ] Chasis: aluminio doblado 2mm o impresión 3D en PETG para prototipo
- [ ] Montaje completo: PCB principal + panel frontal + fuente
- [ ] Cableado interno con conectores JST-XH para mantenibilidad

### Semana 22
- [ ] Test de integración final:
  - 48 horas de operación continua (burn-in test)
  - Prueba de temperatura: no debe superar 60°C en ningún componente
  - Test de ruido de fondo: THD+N < 0.05% a 1kHz (medido con REW)
- [ ] Grabación de demos audio: los 3 modos (FM puro, Vintage Mono, Híbrido)
- [ ] **ENTREGA FINAL**

---

## Criterios de Éxito por Fase

| Fase | Criterio medible |
|---|---|
| 1 | Latencia MIDI < 3ms medida con osciloscopio |
| 2 | PCM5122 genera tono 1kHz limpio, DAC8565 produce voltajes CV correctos ±5mV |
| 3 | 4 voces FM simultáneas, filtro SVF sweep audible sin artefactos, carga CPU < 10% |
| 4 | Parameter locks funcionan en 16 pasos sin glitches |
| 5 | Usuario no técnico puede cambiar preset en < 30s |
| 6 | THD+N < 0.05%, temperatura < 60°C continuo |
