/*
 * Sistemas Embebidos Avanzados II - DSI - EIE - FCEIA - UNR
 * Ejemplo 5: productor y consumidor con una COLA DE MENSAJES
 *
 * - tarea_productora: cada 500 ms genera una medición y la ENVÍA a la cola.
 * - tarea_display:    se bloquea esperando mensajes en la cola y, cuando
 *                     llega uno, lo muestra por consola.
 *
 * No hay variables globales compartidas: la cola COPIA cada mensaje.
 *
 * Para ver qué pasa cuando el consumidor es más lento que el productor,
 * cambiar DEMORA_DISPLAY_MS a 800 y volver a grabar.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define LARGO_COLA              5       // Cantidad máxima de mensajes en la cola

#define PERIODO_PRODUCTOR_MS    500
#define DEMORA_DISPLAY_MS       0       // Probar con 800: display más lento que el productor
#define ESPERA_ENVIO_MS         0       // 0 = si la cola está llena, se descarta la muestra

#define STACK_TAREAS            4096    // En bytes
#define PRIORIDAD_PRODUCTORA    2
#define PRIORIDAD_DISPLAY       2

static const char *TAG = "cola";

// ---------- Mensaje ----------
typedef struct {
    int numero_muestra;
    int temperatura;
    int humedad;
    TickType_t tick_produccion;         // Momento en que se generó el dato
} medicion_t;

static QueueHandle_t cola_mediciones;

// ---------- Tareas ----------
static void tarea_productora(void *parametros)
{
    medicion_t medicion;
    int muestra = 0;
    int perdidas = 0;
    BaseType_t resultado;

    while (1) {
        muestra = muestra + 1;

        // El mensaje se arma en una variable LOCAL de la productora
        medicion.numero_muestra = muestra;
        medicion.temperatura = 20 + (muestra % 10);
        medicion.humedad = 50 + (muestra % 10);
        medicion.tick_produccion = xTaskGetTickCount();

        // xQueueSend COPIA la estructura dentro de la cola
        resultado = xQueueSend(cola_mediciones, &medicion, pdMS_TO_TICKS(ESPERA_ENVIO_MS));

        if (resultado == pdPASS) {
            ESP_LOGI(TAG, "Productora: envia muestra %d (en cola: %d de %d)",
                     muestra, (int) uxQueueMessagesWaiting(cola_mediciones), LARGO_COLA);
        } else {
            perdidas = perdidas + 1;
            ESP_LOGE(TAG, "Productora: COLA LLENA, se pierde la muestra %d (perdidas: %d)",
                     muestra, perdidas);
        }

        vTaskDelay(pdMS_TO_TICKS(PERIODO_PRODUCTOR_MS));
    }
}

static void tarea_display(void *parametros)
{
    medicion_t recibida;
    int antiguedad_ms;

    while (1) {
        // Se bloquea hasta que haya un mensaje. No consume CPU mientras espera.
        xQueueReceive(cola_mediciones, &recibida, portMAX_DELAY);

        antiguedad_ms = (int) ((xTaskGetTickCount() - recibida.tick_produccion) * portTICK_PERIOD_MS);

        ESP_LOGW(TAG, "DISPLAY -> Muestra %3d | T = %d C | H = %d %% | antiguedad %d ms",
                 recibida.numero_muestra, recibida.temperatura, recibida.humedad, antiguedad_ms);

        // Simula un display lento (por ejemplo, un LCD que tarda en refrescar)
        if (DEMORA_DISPLAY_MS > 0) {
            vTaskDelay(pdMS_TO_TICKS(DEMORA_DISPLAY_MS));
        }
    }
}

// ---------- Programa principal ----------
void app_main(void)
{
    // Cola de LARGO_COLA elementos, cada uno del tamaño de una medicion_t
    cola_mediciones = xQueueCreate(LARGO_COLA, sizeof(medicion_t));
    if (cola_mediciones == NULL) {
        ESP_LOGE(TAG, "No se pudo crear la cola");
        return;
    }

    xTaskCreate(tarea_productora, "productora", STACK_TAREAS, NULL, PRIORIDAD_PRODUCTORA, NULL);
    xTaskCreate(tarea_display, "display", STACK_TAREAS, NULL, PRIORIDAD_DISPLAY, NULL);
}
