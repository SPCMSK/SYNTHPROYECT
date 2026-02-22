# Guía de Investigación y Puntos Críticos
## HybridSynth v1.0 — Qué estudiar, qué reforzar y dónde encontrarlo

---

## 1. Puntos a Investigar ANTES de Comenzar el Hardware

### 1.1 Síntesis FM — Teoría y Algoritmos
**Por qué es crítico:** Si no entiendes la matemática FM, el motor de síntesis sonará correcto
accidentalmente o tendrá artefactos que no sabrás cómo corregir.

**Qué investigar:**
- [ ] Modulación de frecuencia en audio: operadores, algoritmos, relación Modulador/Portadora
- [ ] Índice de modulación (I = Dpeak/fm) y su relación con el timbre
- [ ] Retroalimentación de operador (Feedback) y cómo limitar su rango para evitar ruido DC
- [ ] Diferencia entre FM y PM (Phase Modulation) — el Digitone usa PM, no FM

**Recursos:**
- 📖 "Chowning FM Synthesis" — paper original de John Chowning (1973), disponible gratis en CCRMA Stanford
- 📖 "The FM Theory of Musical Instruments" — Julius O. Smith, CCRMA
- 🎥 YouTube: "FM Synthesis Explained" — Yamaha DX7 analysis series por Loopop
- 📦 Código: `fm_synth_teensy` en GitHub (referencia de implementación en C++)
- 📦 STM32 DSP Library (CMSIS-DSP): funciones de punto flotante optimizadas para Cortex-M7

### 1.2 Síntesis por Tabla de Ondas (Wavetable)
**Por qué es crítico:** Es la base de la generación de ondas de sierra, pulso y formas complejas.

**Qué investigar:**
- [ ] Aliasing y el teorema de Nyquist aplicado a síntesis de audio
- [ ] Anti-aliasing en wavetable: Band-Limited Impulse Train (BLIT) y PolyBLEP
- [ ] Interpolación lineal vs cúbica en la lectura de tabla
- [ ] Cómo generar una tabla de 2048 muestras para una onda de sierra perfecta

**Recursos:**
- 📖 "The Art of VA Filter Design" — Vadim Zavalishin (PDF gratuito en Native Instruments)
- 📖 DSPGuide.com — "The Scientist and Engineer's Guide to DSP" (gratuito online)
- 🎥 "PolyBLEP Oscillator" — tutorial por Martin Finke (mti2935.com)
- 📦 GitHub: `Synthesis-Algorithms-C` — colección de algoritmos clásicos en C

### 1.3 Teoría de Filtros Analógicos
**Por qué es crítico:** Necesitas entender el AS3320 a nivel de circuito para calibrarlo y corregir problemas.

**Qué investigar:**
- [ ] Filtro de escalera de transistores (Moog Ladder Filter): teoría de los 4 polos
- [ ] Control de voltaje: cómo un OTA (Operational Transconductance Amplifier) cambia fc con corriente
- [ ] Resonancia y autooscilación: cuándo y por qué el filtro empieza a oscilar solo
- [ ] Coeficiente de temperatura de transistores NPN: por qué la frecuencia de corte se mueve con temperatura
- [ ] Decibelios, frecuencia de corte y la pendiente de -24dB/Oct en la práctica

**Recursos:**
- 📖 "Musical Applications of Microprocessors" — Hal Chamberlin (libro clásico, buscar PDF)
- 📖 "Moog Synthesizer VCF Design" — notas originales de Robert Moog (disponibles en moogfoundation.org)
- 🎥 DIYSOUND.net — tutoriales de construcción de VCF analógico
- 📦 SPICE simulation: LTSpice (gratuito) — simular el circuito del AS3320 antes de soldar

### 1.4 USB Audio Class y USB MIDI Class
**Qué investigar:**
- [ ] Especificación USB MIDI 1.0: paquetes de 4 bytes, CIN codes, cómo funciona el enumeration
- [ ] Diferencia entre USB MIDI 1.0 y 2.0: cuándo importa
- [ ] STM32H7 USB OTG FS: periférico, endpoints, DMA, clock de 48MHz
- [ ] Cómo usar STM32CubeMX para configurar el stack USB CDC o custom class
- [ ] Latencia de USB FS BULK endpoint: por qué es importante el polling interval

**Recursos:**
- 📄 USB MIDI 1.0 Specification — descarga gratuita en usb.org
- 📄 USB 2.0 Specification — usb.org
- 📦 STM32CubeH7 middleware: USB Device library (incluye ejemplo HID y CDC)
- 🎥 "USB on STM32" — serie de tutoriales por Controller Tech en YouTube
- 📦 GitHub: `stm32-usb-midi` — clase USB MIDI para STM32 por varios autores

---

## 2. Puntos a Reforzar Durante el Desarrollo

### 2.1 FreeRTOS en STM32H7 — Lo Que Siempre Falla

**Errores frecuentes que se cometen siempre:**

**Error 1: Stack overflow silencioso**
La tarea de audio o MIDI se queda sin stack y el sistema reinicia silenciosamente.
Solución: activar `configCHECK_FOR_STACK_OVERFLOW = 2` en FreeRTOSConfig.h.
Siempre definir el hook `vApplicationStackOverflowHook()`.

**Error 2: Llamar funciones de FreeRTOS desde una ISR sin el sufijo "FromISR"**
`xQueueSend()` desde una ISR → hard fault inmediato.
Siempre usar `xQueueSendFromISR()` con `pxHigherPriorityTaskWoken`.

**Error 3: Prioridades invertidas**
La tarea de audio debe tener la prioridad más alta del sistema
(excepto la ISR del timer, que es hardware y no compite con FreeRTOS).
Si la tarea de MIDI tiene mayor prioridad que la de audio → glitches de audio.

**Qué estudiar:**
- [ ] Modelo de interrupt/task en FreeRTOS: NVIC priority mapping con `configMAX_SYSCALL_INTERRUPT_PRIORITY`
- [ ] DMA doble-buffer para audio (half/full callback): cómo evitar underruns
- [ ] Mutex vs Semáforo vs Queue para comunicación ISR↔Tarea

**Recursos:**
- 📖 FreeRTOS Documentation oficial — freertos.org/Documentation
- 📖 "Using FreeRTOS on ARM Cortex-M" — capítulo relevante en Mastering FreeRTOS (PDF gratuito)
- 📦 STM32H7 FreeRTOS examples en GitHub

### 2.2 KiCad PCB Design — Lo Que Siempre Genera Errores

**Errores frecuentes:**
1. **Footprint incorrecto**: el IC en el esquemático tiene un footprint que no coincide con el real.
   *Siempre medir el IC físico con calibre antes de mandar a fabricar.*

2. **Falta de vias de drenaje de calor (thermal vias)**: el LT3042 y el LT3094 necesitan un pad térmico
   conectado a GND por vias. Sin ellas, el IC se recalienta y se protege térmicamente apagándose.

3. **Clases de net sin reglas DRC**: las pistas de audio (señal pequeña) deben ser de 0.2mm mínimo,
   pero las pistas de potencia (12V/GND) deben ser ≥ 1mm. Si no defines net classes, el DRC no avisa.

4. **Pads de footprint SMD sin pasta de soldar**: revisar que la capa `F.Paste` esté activa en los footprints
   de los ICs que van a soldarse con stencil.

**Qué estudiar:**
- [ ] KiCad 7/8: workflow completo desde esquemático hasta Gerber
- [ ] Conceptos de PCB layout: impedancia controlada, longitud de pistas diferenciales
- [ ] Cómo leer y crear footprints personalizados
- [ ] Cómo usar el fabrication output wizard de KiCad para JLCPCB

**Recursos:**
- 🎥 "KiCad EDA for Beginners" — FreeCad Tutorials en YouTube (serie completa)
- 🎥 "Phil's Lab" en YouTube — diseño de PCBs para audio y microcontroladores
- 📦 KiCad official library — github.com/KiCad/kicad-footprints
- 📦 JLCPCB KiCad plugin — para exportar directamente desde KiCad a JLCPCB

### 2.3 ARM Cortex-M7 y STM32H7 — Lo Que Siempre Confunde

**Puntos de confusión frecuentes:**

**Cache de instrucciones y datos activo por defecto en H7**
El H743 tiene I-Cache y D-Cache habilitados por defecto desde el boot.
El DMA accede a la RAM directamente **sin pasar por el cache**.
Si escribes datos en RAM desde el CPU y luego el DMA los lee → el DMA ve datos viejos (cache no flusheado).
Solución: `SCB_CleanDCache_by_Addr()` antes de que el DMA lea datos escritos por el CPU.

**Clock domain crossing**
El H743 tiene múltiples dominios de clock (D1, D2, D3). Un periférico en D2 (como el SPI1)
necesita que su clock sea habilitado en `RCC->APB2ENR` del dominio correcto.
STM32CubeMX hace esto automáticamente, pero si configuras registros a mano → silencio total.

**MPU (Memory Protection Unit)**
Si usas FreeRTOS con MPU activado (recomendado para robustez), debes configurar las regiones
de acceso para los buffers de DMA. Sin esto, el acceso al buffer de audio desde la ISR genera HardFault.

**Qué estudiar:**
- [ ] Cache coherency en Cortex-M7: cómo funciona D-Cache con DMA
- [ ] STM32H7 Reference Manual (RM0433): secciones de RCC, DMA, SAI, USB
- [ ] STM32H7 Errata Sheet: bugs conocidos del silicio (importante: algunos lots tienen bugs de USB)

**Recursos:**
- 📄 STM32H743 Reference Manual RM0433 — descarga gratuita en st.com
- 📄 STM32H743 Datasheet — st.com
- 📄 ARM Cortex-M7 Processor Technical Reference Manual — arm.com
- 🎥 "STM32H7 Audio" — serie de videos por STMicroelectronics en YouTube
- 📦 GitHub: `STM32H7-Audio-DMA` — ejemplos de audio con DMA doble buffer

---

## 3. Errores Clásicos que Siempre Ocurren en Este Tipo de Proyecto

### En el Circuito Analógico
- **VCF suena distorsionado a volúmenes normales**: la señal de entrada al AS3320 tiene demasiado nivel. Reducir con atenuador 10:1 en la entrada.
- **Ruido 50Hz o 60Hz (hum)**: tierra de señal conectada a la carcasa en más de un punto → lazo de tierra. Un solo punto de earth ground.
- **El VCF tiene un tono de fondo incluso con gate OFF**: el AS3320 autooscila levemente cuando la resonancia CV tiene voltaje residual. Filtrar con 100nF al final del DAC8564.
- **El THAT2180 distorsiona**: el nivel de entrada lo supera su rango. El THAT2180 acepta máximo +22dBu en su entrada. Verificar que la señal del PCM5242 esté atenuada correctamente.

### En el Firmware
- **USB no reconocido**: `wTotalLength` incorrecto en el Configuration Descriptor. Verificar byte a byte.
- **Silencio total en I2S**: buffer de audio DMA en DTCM — mover a AXI SRAM. O formato I2S incorrecto (MSB justificado vs LSB justificado).
- **ADSR no llega exactamente a 0 o a 1**: acumulación de errores de punto flotante en sumas incrementales. Solución: usar recálculo absoluto con `t = counter/length` en lugar de sumas incrementales.
- **Zipper noise al mover un potenciómetro**: ADC lee ruido de cuantización. Aplicar filtro exponencial `y = 0.05*x + 0.95*y` con actualización cada 1ms.
- **FreeRTOS: hard fault aleatorio**: tarea desbordó su stack. Aumentar stack de las tareas de audio a 4096 words mínimo.

### En el Layout PCB
- **Ruido en la salida de audio que desaparece si tocas el chasis con el dedo**: tierra flotante — verificar que AGND esté conectado al chasis en un punto.
- **DAC8564 produce voltaje incorrecto**: el pin LDAC no está siendo pulsado — los 4 canales están en el input register y nunca se actualizan juntos.
- **PCM5242 sin sonido**: SCK (bit clock) y LRCK (word clock) tienen frecuencias que no calzan con el registro de configuración I2C del PCM5242.

---

## 4. Dónde Obtener Materiales

### Chips Especializados de Synth
- **Alfa Rivas** (alfasynth.com): AS3320, AS2044, AS3360, AS3080 — fabricante ruso de clones de los chips originales CEM y SSM. Envío a Latinoamérica disponible.
- **Electric Druid** (electricdruid.net): TEMPCO resistors, chips especializados, modulación, diseñados para synth DIY.
- **THONK** (thonk.co.uk): componentes eurorack, jacks, encoders, fundas de knobs, panels.
- **Synth DIY Guy** (synthdiyguy.com): componentes europeos, envío confiable.

### PCBs y Fabricación
- **JLCPCB** (jlcpcb.com): 5 PCBs de 10×10cm por ~$5 USD más envío. Ensamblaje disponible.
- **PCBWay** (pcbway.com): alternativa con más opciones de acabado (oro, plata, etc.).
- **Oshpark** (oshpark.com): USA, más caro pero mejor calidad para prototipos críticos.

### Componentes Generales
- **Mouser** (mouser.com): stock completo, envío DHL a Latinoamérica en 3–5 días.
- **DigiKey** (digikey.com): alternativa directa, a veces mejor precio en ciertos componentes.
- **LCSC** (lcsc.com): complemento de bajo costo para componentes básicos (resistores, caps, conectores).

### Recursos de Aprendizaje Gratuitos

| Recurso | Tipo | URL |
|---|---|---|
| CCRMA Digital Audio | Papers y tutoriales | ccrma.stanford.edu |
| DSPGuide | Libro completo online | dspguide.com |
| The Art of VA Filter Design | PDF gratuito | native-instruments website |
| Electro-Music Forum | Comunidad synth DIY | electro-music.com |
| ModWiggler / Muff's | Foro eurorack y DIY | modwiggler.com |
| STM32 Community | Foro oficial ST | community.st.com |
| Phil's Lab | YouTube PCB + firmware | youtube.com/@PhilsLab |
| DIYRE (DIY Recording Equipment) | Blog de diseño de audio | diyrecordingequipment.com |

---

## 5. Recomendaciones Generales

1. **Validar algoritmos en simulación antes del PCB.** Se tom\u00f3 la decisi\u00f3n de ir directamente a STM32H7 PCB (el firmware clave `adsr_engine.c` y `usb_midi_desc.c` ya exist\u00edan). Para proyectos sin firmware previo: simular con STM32CubeIDE en Nucleo-H743ZI antes de fabricar PCB propio.

2. **Comprar al menos 3 unidades del AS3320.** Es el componente con menor disponibilidad y mayor probabilidad de daño por voltaje. No existe alternativa drop-in rápida.

3. **Usar siempre fuente de laboratorio con limitación de corriente** al encender un PCB nuevo por primera vez. Limitarla a 100mA. Si la corriente sube inmediatamente a 100mA antes de cualquier inicialización, hay un cortocircuito.

4. **El osciloscopio es obligatorio.** Sin osciloscopio, depurar el timing del I2S o el SPI es imposible. El Rigol DS1054Z ($250 USD, hackeado a 100MHz) es la referencia estándar DIY.

5. **Documentar cada fallo.** Crear un archivo `BUGS_LOG.md` con fecha, síntoma, causa raíz y solución. En proyectos de esta complejidad, los mismos bugs resurgen semanas después y documentarlos ahorra días.

6. **Pedir revisión de par (peer review) del esquemático** a alguien con experiencia antes de enviar a fabricar. Un segundo par de ojos encuentra errores que llevas semanas mirando sin ver. Publicar en el foro de ModWiggler es una opción válida.

7. **Planificar la calibración como parte del diseño.** El AS3320 y el THAT2180 requieren calibración post-montaje. Incluir trimpots accesibles y documentar el procedimiento de calibración ANTES de cerrar el chasis.

8. **LTSpice antes de soldar.** Para cualquier circuito analógico nuevo (el mixer, el conversor V/I del VCF), simularlo en LTSpice primero. LTSpice es gratuito e incluye los modelos SPICE del LM13700, TL072, NE5532 y hasta del AS3320.
