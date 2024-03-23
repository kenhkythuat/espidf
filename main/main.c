#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include <math.h>
#include "esp_task_wdt.h"
#include <common.h>
#define TAG "----MIAN----"

void app_main()
{
    // configure_led(BLINK_GPIO);

    esim_config();
    config_pwm_50hz();

    //     TaskHandle_t xHandle = NULL;
    //     xTimers[0] = xTimerCreate("bink led", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, vTimerCallback);
    //     xTimerStart(xTimers[0], 0);
    //   xTaskCreate(spwm_task, "spwm_task", 2048, NULL, configMAX_PRIORITIES - 2, NULL);
    //     xTaskCreate(status_led, "status_led", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
}
