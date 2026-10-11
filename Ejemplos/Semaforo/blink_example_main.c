/*
 * Sistemas Embebidos Avanzados II - DSI - EIE - FCEIA - UNR
 * Ejemplo 3: protección de un recurso compartido con un SEMÁFORO BINARIO
 *
 * - tarea_productora: simula un sensor. Cada 300 ms genera una muestra nueva
 *                     y la escribe, campo por campo, en una estructura global.
 * - tarea_display:    cada 100 ms lee la estructura, la muestra por consola
 *                     y verifica que los tres campos sean coherentes entre sí.
 *
 * USAR_SEMAFORO = 0 -> sin protección: aparecen datos inconsistentes.
 * USAR_SEMAFORO = 1 -> con semáforo binario: los datos siempre son coherentes.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#define USAR_SEMAFORO          0       // Cambiar a 1 y volver a grabar

#define PERIODO_PRODUCTOR_MS   300
#define PERIODO_DISPLAY_MS     100
#define DEMORA_ESCRITURA_MS    20      // Simula un acceso lento entre campo y campo

#define STACK_TAREAS           4096    // En bytes
#define PRIORIDAD_PRODUCTORA   2
#define PRIORIDAD_DISPLAY      2

static const char *TAG = "semaforo";

// ---------- Recurso compartido ----------
typedef struct {
    int numero_muestra;
    int temperatura;
    int humedad;
} medicion_t;

// Se inicializa con valores coherentes para la muestra 0
static medicion_t dato_compartido = { 0, 20, 50 };

static SemaphoreHandle_t semaforo;

// ---------- Valores simulados ----------
// Temperatura y humedad dependen solo del número de muestra.
// Así el display puede recalcular qué valores esperaba y detectar errores.
static int calcular_temperatura(int muestra)
{
    return 20 + (muestra % 10);        // 20..29
}

static int calcular_humedad(int muestra)
{
    return 50 + (muestra % 10);        // 50..59
}

// ---------- Acceso al recurso ----------
static void tomar_recurso(void)
{
#if USAR_SEMAFORO
    xSemaphoreTake(semaforo, portMAX_DELAY);   // Bloquea hasta obtenerlo
#endif
}

static void liberar_recurso(void)
{
#if USAR_SEMAFORO
    xSemaphoreGive(semaforo);
#endif
}

// ---------- Tareas ----------
static void tarea_productora(void *parametros)
{
    int muestra = 0;

    while (1) {
        muestra = muestra + 1;

        tomar_recurso();
        // ----- Inicio de la sección crítica -----
        dato_compartido.numero_muestra = muestra;
        vTaskDelay(pdMS_TO_TICKS(DEMORA_ESCRITURA_MS));
        dato_compartido.temperatura = calcular_temperatura(muestra);
        vTaskDelay(pdMS_TO_TICKS(DEMORA_ESCRITURA_MS));
        dato_compartido.humedad = calcular_humedad(muestra);
        // ----- Fin de la sección crítica -----
        liberar_recurso();

        vTaskDelay(pdMS_TO_TICKS(PERIODO_PRODUCTOR_MS));
    }
}

static void tarea_display(void *parametros)
{
    medicion_t copia;
    int lecturas = 0;
    int inconsistentes = 0;

    while (1) {
        // Se copia el dato y se libera el recurso enseguida.
        // La impresión, que es lenta, se hace fuera de la sección crítica.
        tomar_recurso();
        copia = dato_compartido;
        liberar_recurso();

        lecturas = lecturas + 1;

        if (copia.temperatura == calcular_temperatura(copia.numero_muestra) &&
            copia.humedad == calcular_humedad(copia.numero_muestra)) {
            ESP_LOGI(TAG, "Muestra %3d | T = %d C | H = %d %%",
                     copia.numero_muestra, copia.temperatura, copia.humedad);
        } else {
            inconsistentes = inconsistentes + 1;
            ESP_LOGW(TAG, "Muestra %3d | T = %d C | H = %d %%  <<< DATO INCONSISTENTE (%d de %d lecturas)",
                     copia.numero_muestra, copia.temperatura, copia.humedad,
                     inconsistentes, lecturas);
        }

        vTaskDelay(pdMS_TO_TICKS(PERIODO_DISPLAY_MS));
    }
}

// ---------- Programa principal ----------
void app_main(void)
{
#if USAR_SEMAFORO
    semaforo = xSemaphoreCreateBinary();
    if (semaforo == NULL) {
        ESP_LOGE(TAG, "No se pudo crear el semaforo");
        return;
    }
    // Un semáforo binario se crea VACÍO (tomado).
    // Para usarlo como protección, hay que darlo una vez: el recurso queda libre.
    xSemaphoreGive(semaforo);
    ESP_LOGI(TAG, "Modo: CON semaforo binario");
#else
    ESP_LOGI(TAG, "Modo: SIN proteccion");
#endif

    xTaskCreate(tarea_productora, "productora", STACK_TAREAS, NULL, PRIORIDAD_PRODUCTORA, NULL);
    xTaskCreate(tarea_display, "display", STACK_TAREAS, NULL, PRIORIDAD_DISPLAY, NULL);
}
