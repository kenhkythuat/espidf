#include "common.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "nvs_flash.h"
#define TAG "----connectivity----"

int check_esim()
{
    static const char *TAG1 = "---connectivity---";
    esp_log_level_set(TAG1, ESP_LOG_INFO);
    ESP_LOGI(TAG1, "Turn off echo for GSM modem.\n");
    sendData(TAG1, "ATE0\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sendData(TAG1, "AT+CPIN?\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if (strstr(data, "+CPIN: READY") != NULL)
    {
        ESP_LOGI(TAG, "-----------------Subcribe Successed !------------------\n");
    }
    sendData(TAG1, "AT+CREG?\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sendData(TAG1, "AT+CPSI?\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sendData(TAG1, "ATI\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sendData(TAG1, "AT+CICCID\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sendData(TAG1, "AT+CSQ\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sendData(TAG1, "AT+CTZU=1\r\n");
    vTaskDelay(200 / portTICK_PERIOD_MS);
    sendData(TAG1, "AT+CTZR=1\r\n");
    return 1;
}
int enable_mqtt_on_gsm_modem()
{
    sendData(TAG, "AT+CMQTTSTART\r\n");
    vTaskDelay(400 / portTICK_PERIOD_MS);
    if (strstr(data, "+CMQTTSTART: 0") != NULL)
    {
        ESP_LOGI(TAG, "-----------------GOT OK------------------\n");
        return 1;
    }
    return 0;
}
int connect_mqtt_server_by_gsm()
{
    sprintf(AT_COMMAND, "AT+CMQTTCONNECT=0,\"%s:%d\",60,1,\"%s\",\"%s\"\r\n", MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS);
    sendData(TAG, AT_COMMAND);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    if (strstr(data, "+CMQTTCONNECT: 0,0") != NULL)
    {
        ESP_LOGI(TAG, "-----------------GOT OK------------------\n");
        return 1;
    }
    return 0;
}
int subscribe_mqtt_via_gsm()
{
    for (int i = 1; i < NUMBER_LOADS + 1; i++)
    {
        sprintf(AT_COMMAND, "%s/snac/%s/%d", FARM, SERIAL_NUMBER, i);
        sprintf(AT_COMMAND, "AT+CMQTTSUBTOPIC=0,%d,1\r\n", (int)strlen(AT_COMMAND));
        sendData(TAG, AT_COMMAND);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        sprintf(AT_COMMAND, "%s/snac/%s/%d", FARM, SERIAL_NUMBER, i);
        sendData(TAG, AT_COMMAND);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        sendData(TAG, "AT+CMQTTSUB=0\r\n");
        vTaskDelay(200 / portTICK_PERIOD_MS);
        if (strstr(data, "+CMQTTSUB: 0,0") != NULL)
        {
            ESP_LOGI(TAG, "-----------------Subcribe Successed !------------------\n");
        }
    }
    return 1;
}
int publish_mqtt_via_gsm()
{
    return 1;
}
