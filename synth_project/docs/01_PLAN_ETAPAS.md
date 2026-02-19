# Plan de Implementación por Etapas
## Sintetizador Híbrido Digital/Analógico — HybridSynth v1.0

> **Metodología:** Desarrollo incremental con validación de audio al final de cada fase.
> Nunca se avanza a la siguiente etapa hasta que la actual produce audio correcto y medible.

---

## Resumen Visual de Fases

```
FASE 1 ──► FASE 2 ──► FASE 3 ──► FASE 4 ──► FASE 5 ──► FASE 6
KiCad PCB  MCU+DAC    VCF/VCA    USB MIDI   Panel+UI   Chasis
 STM32H7   Digital    Analógico   + Presets  Digitone   Final
  4 sem      4 sem      4 sem       3 sem      3 sem     4 sem
```

**Duración total estimada:** 22 semanas (~5.5 meses con dedicación parcial)

---

## FASE 1 — Diseño Esquemático + PCB en KiCad (STM32H7 directo)
**Duración:** 4 semanas
**Objetivo:** Diseñar y enviar a fabricar el PCB principal con STM32H743 — sin prototipo Teensy.

> **Decisión de diseño:** Se eliminó la fase Teensy. El firmware clave (`adsr_engine.c`,
> `usb_midi_desc.c`) ya está desarrollado. Se va directamente al PCB final en STM32H743.

### Semana 1 — Esquemático: Alimentación + MCU
- [ ] Instalar KiCad 8, configurar librerías, crear proyecto HybridSynth
- [ ] Diseñar bloque de alimentación: USB-C 5V → LT3042 (+5V_ANA) → LT1054 → LT3094 (-5V_ANA) + ADP3335 (+3.3V_DIG)
- [ ] Resistencias 5.1kΩ en CC1/CC2 para señalar 1.5A al cargador USB-C
- [ ] Añadir símbolos custom (AS3320, THAT2180) que no están en librerías estándar
- [ ] Diseñar bloque STM32H743: decoupling pin a pin, cristal 25MHz, VCAP1/VCAP2, SWD
- [ ] ERC del bloque power + MCU — 0 errores antes de continuar

### Semana 2 — Esquemático: Audio, Analógica e Interfaz
- [ ] Diseñar USB-B (MIDI) con USBLC6-2 + polyfuse
- [ ] Diseñar PCM5242 (I2S 32-bit) + DAC8564 (SPI 4ch CV) + buffers OPA2134
- [ ] Diseñar VCF AS3320: V/I converters, tempco, trimpots, atenuadores
- [ ] Diseñar VCA THAT2180 + relay bypass + mezclador NE5532
- [ ] Diseñar bus I2C: PCM5242 (0x4C) + SSD1306 (0x3C) + PCA9685 (0x40) + MCP23017 (0x20)
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
- [ ] PCM5242 genera tono de 1kHz en salida de audio — medir con osciloscopio + auriculares
- [ ] USB MIDI enumera correctamente en Windows/Linux (Product="HybridSynth MIDI")
- [ ] Enviar NoteOn desde Ableton, verificar recepción en STM32 con SWD debugger
- [ ] DAC8564 produce voltajes CV medibles con multímetro (0.0V, 1.25V, 2.5V, 5.0V)

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
- DAC8564 produciendo 4 voltajes CV correc

### Entregables al finalizar
- Primer PCB funcional con sonido FM por I2S
- DAC8564 enviando voltajes CV medibles con multímetro
- USB MIDI reconocido por Ableton en la nueva placa

---

## FASE 3 — Ruta de Audio Analógica (VCF + VCA)
**Duración:** 4 semanas
**Objetivo:** El AS3320 filtrando la señal FM. Sonido tipo Moog audible.

### Semana 9
- [ ] Diseño del circuito VCF con AS3320:
  - Transconductancia de entrada: 1kΩ en cada entrada para evitar saturación
  - Bias de resonancia: trimpot de 10kΩ para calibrar Q máxima
  - Circuito de conversión V→I para el CV de cutoff (DAC8564 → OTA input)
- [ ] Breadboard test del AS3320 con generador de señal externo (osciloscopio)
- [ ] Medir respuesta en frecuencia: debe llegar a -24dB/Oct confirmado

### Semana 10
- [ ] Diseño del circuito VCA con THAT2180:
  - Convertidor dB/V: la envolvente lineal del DAC se convierte a dB
  - Buffer de salida: NE5532 en configuración unity gain buffer
  - Medición de rango dinámico: debe ser > 80dB
- [ ] Integrar VCF + VCA en cadena completa sobre breadboard
- [ ] Inyectar señal FM del PCM5242 al VCF y medir salida post-VCA

### Semana 11
- [ ] Diseñar el mezclador de modo:
  - Switch de bypass del VCF (relay OMRON G6K-2F para conmutación limpia)
  - Entrada externa (jack 6.35mm) para señales modulares externas
  - Mixer sumador con TL072 para combinar FM + analógico en modo híbrido
- [ ] Primera prueba completa de la cadena: FM → VCF → VCA → salida

### Semana 12
- [ ] Calibración del tracking V/Oct del VCF (el pitch del self-oscillation sigue al CV)
- [ ] Ajuste del offset de temperatura del AS3320 con trimpots
- [ ] Medir y documentar respuesta en frecuencia completa con herramienta REW
- [ ] **GATE DE SALIDA:** El sintetizador debe sonar reconociblemente como Moog

### Entregables al finalizar
- Cadena analógica VCF+VCA funcional y calibrada
- Mediciones de respuesta en frecuencia y rango dinámico documentadas
- Audio grabado: sweep de cutoff con resonancia alta (prueba clave)

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
  - Estructura de preset: adsr_params_t + fm_params_t + vcf_params_t
  - Hasta 512 presets en 128MB de flash
  - Función de guardado/carga desde encoders
- [ ] Implementar "Parameter Locks" estilo Digitone:
  - Cada uno de los 16 pasos del secuenciador puede tener valores de CC diferentes
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
| 2 | DAC8564 produce voltajes correctos ±5mV |
| 3 | VCF tracking < ±10 cents en 4 octavas |
| 4 | Parameter locks funcionan en 16 pasos sin glitches |
| 5 | Usuario no técnico puede cambiar preset en < 30s |
| 6 | THD+N < 0.05%, temperatura < 60°C continuo |
