/*
 * Sistemas Embebidos Avanzados II - DSI - EIE - FCEIA - UNR
 * Ejemplo 4: inversión de prioridad. Semáforo binario vs. mutex
 *
 * Tres tareas, todas en el NÚCLEO 1, que se despiertan cada 2 segundos
 * con un pequeño desfase entre ellas:
 *
 *   tarea_productora (prioridad BAJA,  t =  0 ms): toma el recurso y tarda
 *                     100 ms de CPU en escribir una medición.
 *   tarea_display    (prioridad ALTA,  t = 20 ms): quiere leer la medición
 *                     y mide cuánto tiempo tuvo que esperar el recurso.
 *   tarea_calculo    (prioridad MEDIA, t = 40 ms): no usa el recurso;
 *                     hace 500 ms de cálculo intensivo.
 *
 * USAR_MUTEX = 0 -> semáforo binario: el display espera unos 580 ms
 *                   (inversión de prioridad).
 * USAR_MUTEX = 1 -> mutex con herencia de prioridad: el display espera
 *                   unos 80 ms.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#define USAR_MUTEX               0       // Cambiar a 1 y volver a grabar

#define PERIODO_CICLO_MS         2000    // Las tres tareas repiten cada 2 s

#define DESFASE_DISPLAY_MS       20      // El display arranca 20 ms después de la productora
#define DESFASE_CALCULO_MS       40      // El cálculo arranca 40 ms después de la productora

#define TRABAJO_ESCRITURA_MS     50      // CPU entre campo y campo (100 ms en total)
#define TRABAJO_CALCULO_MS       500     // CPU que consume la tarea de prioridad media

#define PRIORIDAD_BAJA           2       // tarea_productora
#define PRIORIDAD_MEDIA          3       // tarea_calculo
#define PRIORIDAD_ALTA           4       // tarea_display

#define NUCLEO_TAREAS            1       // Las tres tareas comparten el núcleo 1
#define STACK_TAREAS             4096    // En bytes

static const char *TAG = "mutex";

// ---------- Recurso compartido ----------
typedef struct {
    int numero_muestra;
    int temperatura;
    int humedad;
} medicion_t;

static medicion_t dato_compartido = { 0, 20, 50 };

static SemaphoreHandle_t proteccion;     // Semáforo binario o mutex, según USAR_MUTEX

// ---------- Trabajo de CPU simulado ----------
// Ocupa la CPU durante 'milisegundos' de ejecución efectiva, en bloques de 1 ms.
// A diferencia de vTaskDelay(), la tarea NO se bloquea: sigue lista y consumiendo CPU.
// Si otra tarea la desaloja, el trabajo pendiente se retoma al volver.
static void trabajo_cpu(int milisegundos)
{
    int i;

    for (i = 0; i < milisegundos; i++) {
        esp_rom_delay_us(1000);          // Espera activa de 1 ms
    }
}

// ---------- Tareas ----------
static void tarea_productora(void *parametros)
{
    int muestra = 0;
    TickType_t ultimo_despertar = xTaskGetTickCount();

    while (1) {
        muestra = muestra + 1;

        xSemaphoreTake(proteccion, portMAX_DELAY);
        // ----- Inicio de la sección crítica -----
        dato_compartido.numero_muestra = muestra;
        trabajo_cpu(TRABAJO_ESCRITURA_MS);
        dato_compartido.temperatura = 20 + (muestra % 10);
        trabajo_cpu(TRABAJO_ESCRITURA_MS);
        dato_compartido.humedad = 50 + (muestra % 10);
        // ----- Fin de la sección crítica -----
        xSemaphoreGive(proteccion);

        ESP_LOGI(TAG, "[BAJA ] productora: libera el recurso (muestra %d)", muestra);

        vTaskDelayUntil(&ultimo_despertar, pdMS_TO_TICKS(PERIODO_CICLO_MS));
    }
}

static void tarea_display(void *parametros)
{
    medicion_t copia;
    int64_t tiempo_antes;
    int64_t tiempo_despues;
    int espera_ms;
    TickType_t ultimo_despertar;

    vTaskDelay(pdMS_TO_TICKS(DESFASE_DISPLAY_MS));
    ultimo_despertar = xTaskGetTickCount();

    while (1) {
        ESP_LOGI(TAG, "[ALTA ] display: pide el recurso");

        tiempo_antes = esp_timer_get_time();          // Tiempo en microsegundos
        xSemaphoreTake(proteccion, portMAX_DELAY);
        tiempo_despues = esp_timer_get_time();

        copia = dato_compartido;
        xSemaphoreGive(proteccion);

        espera_ms = (int) ((tiempo_despues - tiempo_antes) / 1000);

        ESP_LOGW(TAG, "[ALTA ] display: muestra %d | T = %d C | H = %d %% | ESPERO %d ms",
                 copia.numero_muestra, copia.temperatura, copia.humedad, espera_ms);

        vTaskDelayUntil(&ultimo_despertar, pdMS_TO_TICKS(PERIODO_CICLO_MS));
    }
}

static void tarea_calculo(void *parametros)
{
    TickType_t ultimo_despertar;

    vTaskDelay(pdMS_TO_TICKS(DESFASE_CALCULO_MS));
    ultimo_despertar = xTaskGetTickCount();

    while (1) {
        ESP_LOGI(TAG, "[MEDIA] calculo: inicio");
        trabajo_cpu(TRABAJO_CALCULO_MS);
        ESP_LOGI(TAG, "[MEDIA] calculo: fin");

        vTaskDelayUntil(&ultimo_despertar, pdMS_TO_TICKS(PERIODO_CICLO_MS));
    }
}

// ---------- Programa principal ----------
void app_main(void)
{
#if USAR_MUTEX
    proteccion = xSemaphoreCreateMutex();        // El mutex se crea DISPONIBLE
    ESP_LOGI(TAG, "Modo: MUTEX (con herencia de prioridad)");
#else
    proteccion = xSemaphoreCreateBinary();       // El semáforo binario se crea VACÍO...
    if (proteccion != NULL) {
        xSemaphoreGive(proteccion);              // ...por eso se lo da una vez
    }
    ESP_LOGI(TAG, "Modo: SEMAFORO BINARIO");
#endif

    if (proteccion == NULL) {
        ESP_LOGE(TAG, "No se pudo crear la proteccion del recurso");
        return;
    }

    // Las tres tareas se fijan al mismo núcleo. Si pudieran ejecutarse en núcleos
    // distintos, la tarea media no le quitaría la CPU a la baja y no habría inversión.
    xTaskCreatePinnedToCore(tarea_productora, "productora", STACK_TAREAS, NULL,
                            PRIORIDAD_BAJA, NULL, NUCLEO_TAREAS);
    xTaskCreatePinnedToCore(tarea_display, "display", STACK_TAREAS, NULL,
                            PRIORIDAD_ALTA, NULL, NUCLEO_TAREAS);
    xTaskCreatePinnedToCore(tarea_calculo, "calculo", STACK_TAREAS, NULL,
                            PRIORIDAD_MEDIA, NULL, NUCLEO_TAREAS);
}
