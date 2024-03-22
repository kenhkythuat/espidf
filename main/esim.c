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
#include "String.h"
#include "math.h"
#include "nvs_flash.h"

#define TAG "----MQTT Esim----"

#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_2)
#define A7672_PWRKEY (GPIO_NUM_42)
static const int RX_BUF_SIZE = 1024;
void init_uart(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "----------------------Create UART -------------------------------- \n");
}
int sendData(const char *logName, const char *data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    ESP_LOGI(logName, "Wrote %s bytes", data);
    return txBytes;
}
static void configure_output(int num_gpio)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(num_gpio);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(num_gpio, GPIO_MODE_OUTPUT);
}
static void tx_esim(void *arg)
{
    static const char *TX_TASK_TAG = "---WRITE ESIM---";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    // uint8_t *rxdata = (uint8_t *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        // char tempData[10];
        // sprintf(tempData, "AT", temp);
        sendData(TX_TASK_TAG, "ATE0");
        // ESP_LOGI(TX_TASK_TAG, "\n-------------%s-----------------", pumpID);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    // free(rxdata);
}
static void rx_esim(void *arg)
{
    static const char *RX_ESIM_TAG = "RX_TASK_ESIM";
    esp_log_level_set(RX_ESIM_TAG, ESP_LOG_INFO);
    uint8_t *data = (uint8_t *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_PERIOD_MS);
        // ESP_LOGI(RX_ESIM_TAG, "-----------------ham nhan data------------------");
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            ESP_LOGI(RX_ESIM_TAG, "\n-------------Read %d bytes: '%s'-------------------", rxBytes, data);
        }
        // vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    free(data);
}
static void open_simcom(void)
{
    gpio_set_level(A7672_PWRKEY, 1);
    ESP_LOGI(TAG, "-----------------kich hoat module sim------------------");
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(A7672_PWRKEY, 0);
    // ESP_LOGI(TAG, "-----------------kich hoat module sim------------------");
    // vTaskDelay(pdMS_TO_TICKS(500));
}
void esim_config(void)
{
    init_uart();
    configure_output(A7672_PWRKEY);
    open_simcom();
    xTaskCreate(rx_esim, "read_esim", 2048, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(tx_esim, "write_esim", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
}