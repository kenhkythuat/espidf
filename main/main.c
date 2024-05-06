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
    esim_config();
    config_pwm_50hz();
}
