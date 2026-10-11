/*
 * Sistemas Embebidos Avanzados II - DSI - EIE - FCEIA - UNR
 * Ejemplo 2: blink con dos tareas FreeRTOS sobre ESP32-S3
 *
 * - tarea_led:      cambia el estado del LED RGB cada 500 ms.
 * - tarea_mensajes: envía un mensaje por consola cada 1000 ms.
 *
 * app_main() solo inicializa, crea las tareas y retorna.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#define PIN_LED              CONFIG_BLINK_GPIO   // Se configura en menuconfig (48 en la placa N16R8)
#define BRILLO               16                  // 0..255 por color

#define PERIODO_LED_MS       500
#define PERIODO_MENSAJE_MS   1000

#define STACK_TAREAS         4096                // En ESP-IDF el stack se expresa en BYTES
#define PRIORIDAD_LED        2
#define PRIORIDAD_MENSAJES   2

static const char *TAG = "dos_tareas";
static led_strip_handle_t tira_led;

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

static void tarea_led(void *parametros)
{
    int led_encendido = 0;

    while (1) {
        if (led_encendido == 0) {
            led_strip_set_pixel(tira_led, 0, BRILLO, BRILLO, BRILLO);
            led_strip_refresh(tira_led);
            led_encendido = 1;
        } else {
            led_strip_clear(tira_led);
            led_encendido = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(PERIODO_LED_MS));
    }
}

static void tarea_mensajes(void *parametros)
{
    int contador = 0;

    while (1) {
        ESP_LOGI(TAG, "Mensaje %d - tick %lu - nucleo %d",
                 contador, (unsigned long) xTaskGetTickCount(), xPortGetCoreID());
        contador = contador + 1;

        vTaskDelay(pdMS_TO_TICKS(PERIODO_MENSAJE_MS));
    }
}

void app_main(void)
{
    BaseType_t resultado;

    configurar_led();

    resultado = xTaskCreate(tarea_led, "tarea_led", STACK_TAREAS, NULL, PRIORIDAD_LED, NULL);
    if (resultado != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear tarea_led");
    }

    resultado = xTaskCreate(tarea_mensajes, "tarea_mensajes", STACK_TAREAS, NULL, PRIORIDAD_MENSAJES, NULL);
    if (resultado != pdPASS) {
        ESP_LOGE(TAG, "No se pudo crear tarea_mensajes");
    }

    ESP_LOGI(TAG, "Tareas creadas. app_main() termina y main_task se elimina.");
}
