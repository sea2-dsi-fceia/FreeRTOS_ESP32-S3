/*
 * Sistemas Embebidos Avanzados II - DSI - EIE - FCEIA - UNR
 * Ejemplo 6: cola de mensajes + SOFTWARE TIMER
 *
 * - tarea_productora: cada 500 ms genera una medición y la ENVÍA a la cola.
 * - tarea_display:    se bloquea esperando mensajes en la cola y, cuando
 *                     llega uno, lo muestra por consola.
 * - timer_led:        software timer de FreeRTOS que cada 1000 ms ejecuta
 *                     una función que cambia el estado del LED RGB.
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
#include "freertos/timers.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#define PIN_LED                 CONFIG_BLINK_GPIO   // Se configura en menuconfig (48 en la placa N16R8)
#define BRILLO                  16                  // 0..255 por color
#define PERIODO_LED_MS          1000

#define LARGO_COLA             5       // Cantidad máxima de mensajes en la cola

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

// ---------- LED y timer ----------
static led_strip_handle_t tira_led;
static TimerHandle_t timer_led;
static int led_encendido = 0;            // Solo la usa la función del timer

static void configurar_led(void)
{
    led_strip_config_t config_tira = {
        .strip_gpio_num = PIN_LED,
        .max_leds = 1,                           // La placa tiene un solo LED
    };
    led_strip_rmt_config_t config_rmt = {
        .resolution_hz = 10 * 1000 * 1000,       // 10 MHz: 0,1 us por tick del RMT
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&config_tira, &config_rmt, &tira_led));
    led_strip_clear(tira_led);
}

// Función de callback del timer.
// NO es una tarea: la ejecuta la tarea de servicio de timers de FreeRTOS
// ("Tmr Svc") cada vez que vence el período. Debe ser corta y no bloquearse.
static void callback_timer_led(TimerHandle_t timer)
{
    if (led_encendido == 0) {
        led_strip_set_pixel(tira_led, 0, BRILLO, BRILLO, BRILLO);
        led_strip_refresh(tira_led);
        led_encendido = 1;
    } else {
        led_strip_clear(tira_led);
        led_encendido = 0;
    }
}

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

    // LED y timer
    configurar_led();

    timer_led = xTimerCreate("timer_led",                    // Nombre (para depuración)
                             pdMS_TO_TICKS(PERIODO_LED_MS),  // Período
                             pdTRUE,                         // pdTRUE = periódico (auto-reload)
                             NULL,                           // ID del timer (no se usa)
                             callback_timer_led);            // Función a ejecutar
    if (timer_led == NULL) {
        ESP_LOGE(TAG, "No se pudo crear el timer");
        return;
    }

    // Crear el timer no lo arranca: hay que iniciarlo
    if (xTimerStart(timer_led, 0) != pdPASS) {
        ESP_LOGE(TAG, "No se pudo iniciar el timer");
    }
}
