# FreeRTOS_ESP32-S3

Ejemplos de **FreeRTOS sobre ESP32-S3** con **ESP-IDF** y **VS Code**.

| | |
|---|---|
| **Asignatura** | Sistemas Embebidos Avanzados II |
| **Departamento** | Departamento de Sistemas e Informática — EIE — FCEIA — UNR |
| **Docente** | Prof. Ing. Daniel Márquez |
| **Contacto** | [dmarquez@fceia.unr.edu.ar](mailto:dmarquez@fceia.unr.edu.ar) |

*EIE: Escuela de Ingeniería Electrónica — FCEIA: Facultad de Ciencias Exactas, Ingeniería y Agrimensura — UNR: Universidad Nacional de Rosario.*

---

## Contenido del repositorio

```
FreeRTOS_ESP32-S3/
├── README.md
├── Guia de Instalacion ESP-IDF ESP32-S3/
│   └── SEA2_Guia_Instalacion_ESP-IDF_ESP32-S3.pdf     Instalación del entorno y primer proyecto
├── Guia de estudio/
│   └── SEA2_FreeRTOS_ESP32-S3_Guia_de_estudio.pdf      Guía completa: entorno, FreeRTOS SMP y ejemplos
├── Proyecto_FreeRTOS_ESP3-S3/
│   └── ESP32_FreeRTOS_blink/                           Proyecto ESP-IDF base (se abre en VS Code)
│       └── main/
│           └── blink_example_main.c                    ← aquí se coloca el código de cada ejemplo
└── Ejemplos/
    ├── Blink/                          blink_example_main.c   Ejemplo 1
    ├── Blink con dos tareas/           blink_example_main.c   Ejemplo 2
    ├── Semaforo/                       blink_example_main.c   Ejemplo 3
    ├── Mutex/                          blink_example_main.c   Ejemplo 4
    ├── Cola de mensajes/               blink_example_main.c   Ejemplo 5
    └── Cola de mensajes y Timer/       blink_example_main.c   Ejemplo 6
```

### Documentación

- 📘 [**Guía de instalación**](Guia%20de%20Instalacion%20ESP-IDF%20ESP32-S3/SEA2_Guia_Instalacion_ESP-IDF_ESP32-S3.pdf): instalación de VS Code, la extensión ESP-IDF y ESP-IDF con el EIM, y creación del primer proyecto.
- 📗 [**Guía de estudio**](Guia%20de%20estudio/SEA2_FreeRTOS_ESP32-S3_Guia_de_estudio.pdf): material de estudio de la asignatura. Incluye la instalación, la configuración del proyecto y de FreeRTOS (`menuconfig`, `sdkconfig`, `FreeRTOSConfig.h`), FreeRTOS en dos núcleos (SMP) y el análisis detallado de cada ejemplo, con actividades propuestas.

**Leer la guía de estudio antes de trabajar con los ejemplos.**

---

## Hardware

- Placa **ESP32-S3 N16R8** con LED RGB integrado (WS2812 en el **GPIO 48**).
- Cable USB **de datos** (no solo de carga).

## Software

- Windows 10/11 de 64 bits.
- **VS Code** + extensión **ESP-IDF** (Espressif Systems).
- **ESP-IDF** instalado con el **ESP-IDF Installation Manager (EIM)**, opción *Easy Installation*.
- Ejemplos probados con **ESP-IDF v6.1**.

### Instalación (resumen)

El paso a paso está en la **guía de instalación**.

1. Instalar **VS Code**.
2. Instalar la extensión **ESP-IDF** desde el panel de Extensiones (`Ctrl+Shift+X`).
3. Descargar e instalar el **EIM** desde la [guía oficial de Espressif para Windows](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/windows-setup.html) y ejecutar **Start Easy Installation**.
4. Cerrar y volver a abrir VS Code.

> Si al abrir el instalador desde VS Code aparece **"EIM not found"**, es porque el EIM se instala por separado (paso 3).

---

## Puesta en marcha (una sola vez)

### 1. Descargar el repositorio

Descargarlo con **Code > Download ZIP** (o clonarlo con GitHub Desktop) y ubicarlo en una ruta **sin espacios ni acentos**, por ejemplo:

```
C:\esp\FreeRTOS_ESP32-S3\
```

> ⚠️ ESP-IDF no compila en rutas con espacios o acentos. Evitar, por ejemplo, `C:\Users\José Pérez\Escritorio\...`.

### 2. Abrir el proyecto en VS Code

**File > Open Folder** y elegir la carpeta del proyecto (no la raíz del repositorio):

```
C:\esp\FreeRTOS_ESP32-S3\Proyecto_FreeRTOS_ESP3-S3\ESP32_FreeRTOS_blink
```

### 3. Configurar el entorno

En el panel **ESP-IDF: Explorer > Commands**:

1. **Select current ESP-IDF version**: elegir la versión instalada en la PC.
2. **Set Espressif Device Target**: `esp32s3` y, si lo pregunta, *ESP32-S3 chip (via builtin USB-JTAG)*.
3. **Select Flash Method**: `UART`.
4. **Select Port to Use** y **Select Monitor Port to Use**: el COM de la placa (ambos iguales).

> El proyecto trae la configuración de la PC del docente (versión de ESP-IDF y puerto COM). Los pasos 1 y 4 la reemplazan por la de cada PC.

### 4. Conexión de la placa y monitor serie

- Conectar la placa por el conector rotulado **UART** (o **COM**): la consola de ESP-IDF sale por la UART0, que va a ese conector.
- El número de COM se ve en el **Administrador de dispositivos > Puertos (COM y LPT)**.
- Los mensajes se ven en el **monitor**, que se abre como una terminal en el panel inferior de VS Code. Si se usa solo *Build* o *Flash*, el monitor **no** se abre: usar **Build, Flash and Monitor** o **Monitor Device**.

### 5. Verificar el pin del LED

El LED RGB de la placa está en el **GPIO 48**. El proyecto ya trae ese valor en `sdkconfig.defaults.esp32s3`. Para verificarlo: **SDK Configuration Editor (menuconfig)**, buscar `Blink`:

- **Blink LED type:** LED strip
- **LED strip backend peripheral:** RMT
- **Blink GPIO number:** **48**

### 6. Primera prueba

Ejecutar **Build, Flash and Monitor**. La primera compilación demora varios minutos.

El LED debe parpadear en blanco una vez por segundo y en el monitor deben aparecer los mensajes `Turning the LED ON/OFF`. Este es el **Ejemplo 1**.

---

## Cómo probar cada ejemplo

Todos los ejemplos se ejecutan sobre **el mismo proyecto**. Cada carpeta de `Ejemplos/` contiene un archivo `blink_example_main.c` que reemplaza al del proyecto.

Para cada ejemplo, repetir estos pasos **siempre en el mismo orden**:

1. Abrir la carpeta del ejemplo dentro de `Ejemplos/`.
2. Llevar su código al proyecto, de una de estas dos formas:
   - **Copiar el contenido:** abrir el `blink_example_main.c` del ejemplo, copiar todo (`Ctrl+A`, `Ctrl+C`) y pegarlo en `main/blink_example_main.c` del proyecto, **reemplazando todo** lo que había; o
   - **Reemplazar el archivo:** copiar el `blink_example_main.c` del ejemplo y pegarlo en la carpeta `main/` del proyecto, aceptando sobrescribir.
3. **Guardar** (`Ctrl+S`) si se editó desde VS Code.
4. Ejecutar **Full Clean** (ESP-IDF: Explorer > Commands).
5. Ejecutar **Build, Flash and Monitor**.
6. Verificar en el monitor que la **etiqueta** de los mensajes corresponda al ejemplo (ver tabla).

```
Ejemplos/<ejemplo>/blink_example_main.c   ──►   Proyecto_FreeRTOS_ESP3-S3/ESP32_FreeRTOS_blink/main/blink_example_main.c
```

> ⚠️ **No renombrar ni mover** `main/blink_example_main.c`: es el archivo que compila el proyecto.

> 🔄 El **Full Clean** fuerza una compilación completa y evita grabar por error el binario del ejemplo anterior. La compilación demora más, pero es la forma segura de cambiar de ejemplo.

> 💾 Al reemplazar el archivo se pierde el código anterior. Si se modificó un ejemplo para una actividad, guardar una copia antes de reemplazarlo.

---

## Ejemplos

| # | Carpeta | Tema | Etiqueta en el monitor | Guía |
|---|---|---|---|---|
| 1 | `Blink` | `app_main()` como tarea | `example:` | Sección 10 |
| 2 | `Blink con dos tareas` | Creación de tareas | `dos_tareas:` | Sección 11 |
| 3 | `Semaforo` | Recurso compartido y semáforo binario | `semaforo:` | Sección 12 |
| 4 | `Mutex` | Inversión de prioridad y mutex | `mutex:` | Sección 13 |
| 5 | `Cola de mensajes` | Cola de mensajes (productor/consumidor) | `cola:` | Sección 14 |
| 6 | `Cola de mensajes y Timer` | Cola de mensajes + software timer | `cola:` | Sección 15 |

### Ejemplo 1 — Blink (`app_main()` como tarea)
Ejemplo *blink* oficial de ESP-IDF, sin modificaciones. Aunque el código no crea tareas, ya corre sobre FreeRTOS: `app_main()` se ejecuta dentro de una tarea, en el **núcleo 0**, y se bloquea con `vTaskDelay()`.

**Qué observar:** el LED parpadea cada 1 s. Al inicio aparecen `main_task: Started on CPU0` y `main_task: Calling app_main()`.

### Ejemplo 2 — Blink con dos tareas
Una tarea cambia el LED cada 500 ms y otra envía un mensaje por consola cada 1 s. `app_main()` solo crea las tareas y retorna.

**Qué observar:** el LED parpadea cada 500 ms. Los mensajes muestran el tick (100 ticks entre mensajes) y el **núcleo** en que corre la tarea, que puede variar entre 0 y 1.

### Ejemplo 3 — Semáforo
Una tarea productora escribe una medición campo por campo en una estructura compartida. Una tarea display la lee y verifica su coherencia.

**Qué observar:** cambiar `#define USAR_SEMAFORO` y volver a grabar.
- `0` → sin protección: aparecen en amarillo líneas con `DATO INCONSISTENTE`. El programa no se cuelga: **lo que falla es el dato**.
- `1` → con semáforo binario: todas las lecturas son coherentes.

### Ejemplo 4 — Mutex
Tres tareas de prioridad baja, media y alta, fijadas al **núcleo 1**. La baja tiene el recurso, la alta lo espera y la media le quita la CPU a la baja.

**Qué observar:** cambiar `#define USAR_MUTEX` y volver a grabar. En **ambos modos los datos son correctos**: lo que cambia es **cuánto espera** la tarea de prioridad alta (`ESPERO ... ms`).
- `0` → semáforo binario: espera larga. `calculo: inicio/fin` aparece **antes** de la línea del display (**inversión de prioridad**).
- `1` → mutex: espera corta. La línea del display aparece **antes** del cálculo (**herencia de prioridad**).

Observar a partir del segundo o tercer ciclo.

### Ejemplo 5 — Cola de mensajes
La productora envía mediciones a una cola cada 500 ms. El display se bloquea esperando mensajes y los muestra al llegar.

**Qué observar:**
- Con `DEMORA_DISPLAY_MS 0`: la cola no acumula (`en cola: 0`) y la antigüedad es de ~0 ms.
- Con `DEMORA_DISPLAY_MS 800`: la cola se llena hasta 5, la antigüedad crece, aparecen en rojo los mensajes `COLA LLENA` y hay saltos en la numeración.

### Ejemplo 6 — Cola de mensajes y Timer
El ejemplo 5 más un **software timer** de FreeRTOS que cambia el LED cada 1 s, sin crear una tarea para ello.

**Qué observar:** la misma salida que el ejemplo 5, y el LED 1 s encendido y 1 s apagado.

---

## Problemas frecuentes

| Síntoma | Solución |
|---|---|
| `EIM not found` | Instalar el EIM por separado (ver Instalación). |
| No aparece ninguna versión de ESP-IDF | Instalar ESP-IDF con el EIM y reiniciar VS Code. |
| Errores de compilación con rutas extrañas | El repositorio está en una ruta con espacios o acentos. Moverlo, por ejemplo, a `C:\esp\`. |
| El monitor muestra mensajes del ejemplo anterior | No se guardó el archivo o se grabó el binario anterior: guardar, **Full Clean** y volver a compilar. |
| No se ven mensajes, pero el LED TX de la placa destella | El monitor no está abierto o está en otro COM. Abrir **Monitor Device** y revisar **Select Monitor Port to Use**. |
| El LED RGB no cambia o queda fijo en un color | Pin incorrecto: configurar **GPIO 48** en menuconfig. |
| No graba (*Failed to connect*) | Mantener **BOOT**, presionar y soltar **RST**, soltar **BOOT** y reintentar. |
| `Access denied` en el COM | Cerrar otros programas que usen el puerto (Arduino IDE, PuTTY, otro monitor). |

Más casos en la sección 17 de la guía de estudio.

---

## Referencias

- [ESP-IDF — Installation of ESP-IDF and Tools on Windows](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/windows-setup.html)
- [ESP-IDF — FreeRTOS Overview (ESP32-S3)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/freertos.html)
- [ESP-IDF — FreeRTOS (IDF) y cambios SMP](https://docs.espressif.com/projects/esp-idf/en/stable/esp32s3/api-reference/system/freertos_idf.html)
- [FreeRTOS — documentación oficial](https://www.freertos.org)

El ejemplo 1 y el proyecto base se basan en el ejemplo *get-started/blink* de ESP-IDF (dominio público / CC0).

---

Sistemas Embebidos Avanzados II — Departamento de Sistemas e Informática — EIE — FCEIA — UNR
Prof. Ing. Daniel Márquez — [dmarquez@fceia.unr.edu.ar](mailto:dmarquez@fceia.unr.edu.ar)
