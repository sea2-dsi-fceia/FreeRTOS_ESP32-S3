# FreeRTOS_ESP32-S3

Ejemplos de **FreeRTOS sobre ESP32-S3** con **ESP-IDF** y **VS Code**.

**Sistemas Embebidos Avanzados II**
Departamento de Sistemas e Informática — Escuela de Ingeniería Electrónica
Facultad de Ciencias Exactas, Ingeniería y Agrimensura — Universidad Nacional de Rosario

Prof. Ing. Daniel Márquez

---

## Contenido del repositorio

```
FreeRTOS_ESP32-S3/
├── README.md
├── docs/
│   └── SEA2_FreeRTOS_ESP32-S3_Guia_de_estudio.pdf
└── ejemplos/
    ├── ej1_blink_original.c
    ├── ej2_dos_tareas.c
    ├── ej3_semaforo_binario.c
    ├── ej4_mutex_inversion_prioridad.c
    ├── ej5_cola_mensajes.c
    └── ej6_cola_y_timer.c
```

La **guía de estudio** (`docs/`) contiene la instalación completa, la explicación de FreeRTOS en ESP-IDF y en dos núcleos, y el análisis detallado de cada ejemplo. **Leerla antes de empezar.**

## Hardware

- Placa **ESP32-S3 N16R8** con LED RGB integrado (WS2812 en el **GPIO 48**).
- Cable USB **de datos** (no solo de carga).

## Requisitos

- Windows 10/11 de 64 bits.
- **VS Code** + extensión **ESP-IDF** (Espressif Systems).
- **ESP-IDF** instalado con el **ESP-IDF Installation Manager (EIM)**, opción *Easy Installation*.

El paso a paso está en la sección 3 de la guía. Resumen:

1. Instalar VS Code.
2. Instalar la extensión **ESP-IDF** desde el panel de Extensiones.
3. Descargar e instalar el **EIM** desde la [guía oficial de Espressif para Windows](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/windows-setup.html) y ejecutar **Start Easy Installation**.
4. Reiniciar VS Code y, en **ESP-IDF: Explorer > Select current ESP-IDF version**, elegir la versión instalada.

> Si al abrir el instalador desde VS Code aparece **"EIM not found"**, es porque el EIM se instala por separado (paso 3).

> **Importante:** usar rutas **sin espacios ni acentos**, por ejemplo `C:\esp\`.

---

## Preparación del proyecto base (una sola vez)

Todos los ejemplos se ejecutan sobre **un mismo proyecto**, creado a partir del ejemplo oficial *blink*.

1. **ESP-IDF: Explorer > Advanced > New Project Wizard**.
2. Buscar `blink` (o **ESP-IDF Examples > get-started > blink**) y completar:

   | Campo | Valor |
   |---|---|
   | Project Name | `blink` |
   | Project directory | `C:\esp` |
   | ESP-IDF Target | `esp32s3` |
   | ESP-IDF Board | ESP32-S3 chip (via builtin USB-JTAG) |
   | Serial port | El COM de la placa |
   | Component directory | (vacío) |

3. **Create Project** y abrir el proyecto.
4. Abrir **SDK Configuration Editor (menuconfig)**, buscar `Blink` y configurar:
   - **Blink LED type:** LED strip
   - **LED strip backend peripheral:** RMT
   - **Blink GPIO number:** **48** ⚠️ (el ejemplo trae 38, que no corresponde a esta placa)

   y hacer clic en **Save**.
5. Compilar y grabar con **Build, Flash and Monitor**. El LED debe parpadear en blanco una vez por segundo y en el monitor deben aparecer los mensajes `Turning the LED ON/OFF`. Este es el **Ejemplo 1**.

### Conexión y monitor serie

- Conectar la placa por el conector rotulado **UART** (o **COM**). La consola de ESP-IDF sale por la UART0, que va a ese conector.
- Ver en el **Administrador de dispositivos > Puertos (COM y LPT)** qué COM se asignó.
- En ESP-IDF: Explorer, elegir ese COM en **Select Port to Use** y también en **Select Monitor Port to Use**.
- Los mensajes se ven en el **monitor**, que se abre como una terminal en el panel inferior de VS Code. Si se usa solo *Build* o *Flash*, el monitor no se abre: usar **Build, Flash and Monitor** o **Monitor Device**.

---

## Cómo probar cada ejemplo

Para cada ejemplo, repetir estos pasos **siempre en el mismo orden**:

1. Abrir el archivo del ejemplo en la carpeta `ejemplos/` y **copiar todo su contenido**.
2. Pegarlo en `main/blink_example_main.c` del proyecto, **reemplazando todo** lo que había.
3. **Guardar** el archivo (`Ctrl+S`).
4. Ejecutar **Full Clean** (ESP-IDF: Explorer > Commands).
5. Ejecutar **Build, Flash and Monitor**.
6. Verificar en el monitor que la **etiqueta** de los mensajes corresponda al ejemplo (ver tabla).

> ⚠️ **No renombrar ni mover** `blink_example_main.c`: es el archivo que compila el proyecto.

> 💾 Al pegar un ejemplo nuevo se pierde el anterior. Si se modificó un ejemplo para una actividad, guardar una copia antes de reemplazarlo.

> 🔄 El **Full Clean** fuerza una compilación completa y evita grabar por error el binario del ejemplo anterior. La compilación demora más, pero es la forma segura de cambiar de ejemplo.

---

## Ejemplos

| # | Archivo | Tema | Etiqueta en el monitor |
|---|---|---|---|
| 1 | `ej1_blink_original.c` | `app_main()` como tarea | `example:` |
| 2 | `ej2_dos_tareas.c` | Creación de tareas | `dos_tareas:` |
| 3 | `ej3_semaforo_binario.c` | Recurso compartido y semáforo binario | `semaforo:` |
| 4 | `ej4_mutex_inversion_prioridad.c` | Inversión de prioridad y mutex | `mutex:` |
| 5 | `ej5_cola_mensajes.c` | Cola de mensajes (productor/consumidor) | `cola:` |
| 6 | `ej6_cola_y_timer.c` | Cola de mensajes + software timer | `cola:` |

### Ejemplo 1 — Blink original (`app_main()` como tarea)
Es el ejemplo *blink* de ESP-IDF sin modificaciones. Aunque el código no crea tareas, ya corre sobre FreeRTOS: `app_main()` se ejecuta dentro de una tarea (en el **núcleo 0**) y se bloquea con `vTaskDelay()`.

**Qué observar:** el LED parpadea cada 1 s; al inicio aparecen `main_task: Started on CPU0` y `main_task: Calling app_main()`.

### Ejemplo 2 — Dos tareas
Una tarea cambia el LED cada 500 ms y otra envía un mensaje por consola cada 1 s. `app_main()` solo crea las tareas y retorna.

**Qué observar:** el LED parpadea cada 500 ms; los mensajes muestran el tick (100 ticks entre mensajes) y el **núcleo** en que corre la tarea, que puede variar.

### Ejemplo 3 — Semáforo binario
Una tarea productora escribe una medición campo por campo en una estructura compartida; una tarea display la lee y verifica su coherencia.

**Qué observar:** cambiar `#define USAR_SEMAFORO` y volver a grabar.
- `0` → sin protección: aparecen en amarillo líneas con `DATO INCONSISTENTE`. El programa no se cuelga: **lo que falla es el dato**.
- `1` → con semáforo: todas las lecturas son coherentes.

### Ejemplo 4 — Inversión de prioridad y mutex
Tres tareas de prioridad baja, media y alta, fijadas al **núcleo 1**. La baja tiene el recurso, la alta lo espera y la media le quita la CPU a la baja.

**Qué observar:** cambiar `#define USAR_MUTEX` y volver a grabar. En **ambos modos los datos son correctos**; lo que cambia es **cuánto espera** la tarea de prioridad alta (`ESPERO ... ms`):
- `0` → semáforo binario: espera larga; `calculo: inicio/fin` aparece **antes** de la línea del display (inversión de prioridad).
- `1` → mutex: espera corta; la línea del display aparece **antes** del cálculo (herencia de prioridad).

Observar a partir del segundo o tercer ciclo.

### Ejemplo 5 — Cola de mensajes
La productora envía mediciones a una cola cada 500 ms; el display se bloquea esperando mensajes y los muestra al llegar.

**Qué observar:**
- Con `DEMORA_DISPLAY_MS 0`: la cola no acumula (`en cola: 0`) y la antigüedad es ~0 ms.
- Con `DEMORA_DISPLAY_MS 800`: la cola se llena hasta 5, la antigüedad crece, aparecen en rojo los mensajes `COLA LLENA` y hay saltos en la numeración.

### Ejemplo 6 — Cola de mensajes + software timer
El ejemplo 5 más un **software timer** de FreeRTOS que cambia el LED cada 1 s, sin crear una tarea para ello.

**Qué observar:** la misma salida que el ejemplo 5 y el LED 1 s encendido, 1 s apagado.

---

## Problemas frecuentes

| Síntoma | Solución |
|---|---|
| `EIM not found` | Instalar el EIM por separado (ver Requisitos). |
| No aparece ninguna versión de ESP-IDF | Instalar ESP-IDF con el EIM y reiniciar VS Code. |
| Hay mensajes del ejemplo anterior | No se guardó el archivo o se grabó el binario viejo: guardar, **Full Clean** y volver a compilar. |
| No se ven mensajes (el LED TX de la placa destella) | El monitor no está abierto o está en otro COM. Abrir **Monitor Device** y revisar **Select Monitor Port to Use**. |
| El LED RGB no cambia o queda fijo en un color | Pin incorrecto: configurar **GPIO 48** en menuconfig. |
| No graba (*Failed to connect*) | Mantener **BOOT**, presionar y soltar **RST**, soltar **BOOT** y reintentar. |
| `Access denied` en el COM | Cerrar otros programas que usen el puerto (Arduino IDE, PuTTY, otro monitor). |
| Errores de compilación con rutas raras | Mover el proyecto a una ruta sin espacios ni acentos. |

Más casos en la sección 17 de la guía.

---

## Referencias

- [ESP-IDF — Installation on Windows](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/windows-setup.html)
- [ESP-IDF — FreeRTOS Overview (ESP32-S3)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/freertos.html)
- [ESP-IDF — FreeRTOS (IDF) y cambios SMP](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/freertos_idf.html)
- [FreeRTOS — documentación oficial](https://www.freertos.org)

El ejemplo 1 es el ejemplo *get-started/blink* de ESP-IDF (dominio público / CC0).
