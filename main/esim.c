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
#ifndef FARM
#define FARM "gateway-agriconnect"
#endif

// Serial number. Must be lower case.
#ifndef SERIAL_NUMBER
#define SERIAL_NUMBER "sw000156"
#endif
#define MQTT_CLIENT_ID SERIAL_NUMBER
#define MQTT_PORT 1883

/** MQTT
 * Global broker: mqtt.agriconnect.vn
 */
#define MQTT_HOST "tcp://mqtt.agriconnect.vn" // MQTT broker
#define MQTT_USER "mqttnode"                  // User - connect to MQTT broker
#define MQTT_PASS "congamo"
char AT_RESET[] = "AT+CRESET\r\n";
char AT_CHECK_A76XX[] = "AT\r\n";
char AT_CHECK_ESIM[] = "AT+CGREG?\r\n";
char AT_START_MQTT[] = "AT+CMQTTSTART\r\n";
char AT_ACQUIRE_CLIENT[] = "AT+CMQTTACCQ=0,\"%s\",0\r\n";
char AT_CONNECT_MQTT[] = "AT+CMQTTCONNECT=0,\"%s:%d\",60,1,\"%s\",\"%s\"\r\n";
char AT_SET_PUBLISH_TOPIC[] = "AT+CMQTTTOPIC=0,%d\r\n";
char AT_SET_PUBLISH_PAYLOAD[] = "AT+CMQTTPAYLOAD=0,%d\r\n";
char AT_PUBLISH[] = "AT+CMQTTPUB=0,1,60\r\n"; // Password - connect to MQTT broker
char AT_SUBCRIBE[] = "AT+CMQTTSUB=0\r\n";
char AT_SET_SUBCRIBE_0_9_TOPIC[] = "AT+CMQTTSUBTOPIC=0,35,1\r\n";
char AT_SET_SUBCRIBE_10_18_TOPIC[] = "AT+CMQTTSUBTOPIC=0,36,1\r\n";
char AT_SUBCRIBE_TOPIC[] = "%s%d\r\n";
char AT_COMMAND[100];
char AT_INFORM_PAYLOAD[] = "{%d:%d}\r\n";
char AT_COMMAND[100];

#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_2)
#define A7672_PWRKEY (GPIO_NUM_42)
#define RELAY_1 (GPIO_NUM_9)
#define RELAY_2 (GPIO_NUM_10)
#define RELAY_3 (GPIO_NUM_11)
#define RELAY_4 (GPIO_NUM_12)
unsigned int GPIO_LOAD_PIN[10] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4};

static const int RX_BUF_SIZE = 1024;
bool isPBDONE = false;
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
        if (isPBDONE == true)
        {
            sendData(TX_TASK_TAG, "AT\r\n");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, "ATE0\r\n");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, "AT+CSQ\r\n");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, "AT+CMQTTSTOP\r\n");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, "AT+CMQTTSTART\r\n");
            vTaskDelay(2000 / portTICK_PERIOD_MS);

            sprintf(AT_COMMAND, AT_ACQUIRE_CLIENT, MQTT_CLIENT_ID);
            sendData(TX_TASK_TAG, AT_COMMAND);
            vTaskDelay(2000 / portTICK_PERIOD_MS);

            sprintf(AT_COMMAND, AT_CONNECT_MQTT, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS);
            sendData(TX_TASK_TAG, AT_COMMAND);
            vTaskDelay(2000 / portTICK_PERIOD_MS);

            sprintf(AT_COMMAND, AT_ACQUIRE_CLIENT, MQTT_CLIENT_ID);
            sendData(TX_TASK_TAG, AT_COMMAND);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            for (int i = 1; i < NUMBER_LOADS + 1; i++)
            {
                if (i > 9)
                {
                    sprintf(AT_COMMAND, AT_SET_SUBCRIBE_10_18_TOPIC);
                    sendData(TX_TASK_TAG, AT_COMMAND);
                }
                else
                {
                    sprintf(AT_COMMAND, AT_SET_SUBCRIBE_0_9_TOPIC);
                    sendData(TX_TASK_TAG, AT_COMMAND);
                }
                vTaskDelay(200 / portTICK_PERIOD_MS);
                sprintf(AT_COMMAND, AT_SUBCRIBE_TOPIC, MQTT_TOPIC_ACTUATOR_CONTROL, i);
                sendData(TX_TASK_TAG, AT_COMMAND);
                vTaskDelay(200 / portTICK_PERIOD_MS);
                sendData(AT_COMMAND, AT_SUBCRIBE);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            vTaskSuspend(NULL);
        }
    }
    // free(rxdata);
}
static void rx_esim(void *arg)
{
    static const char *RX_ESIM_TAG = "RX_TASK_ESIM";
    esp_log_level_set(RX_ESIM_TAG, ESP_LOG_INFO);
    char *data = (char *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_PERIOD_MS);
        // ESP_LOGI(RX_ESIM_TAG, "-----------------ham nhan data------------------");
        if (rxBytes > 0)
        {
            data[rxBytes] = 0;
            ESP_LOGI(RX_ESIM_TAG, "\n-------------Read %d bytes: '%s'-------------------", rxBytes, data);
            if (strstr(data, "PB DONE"))
            {
                isPBDONE = true;
            }
            for (int i = 0; i < rxBytes; i++)
            {
                if ((char)data[i] == '1' && (char)data[i + 1] == '5' && (char)data[i + 2] == '6')
                {
                    static int num_load = 0;
                    num_load = data[i + 4] - 48;
                    printf("-----------RELAY  %d DA tim thay-----------\r\n", num_load);
                    if (data[i + 31] == 49)
                    {
                        printf("-----------ON RELAY %d-----------\r\n", data[i + 31]);
                        gpio_set_level(GPIO_LOAD_PIN[num_load - 1], 1);
                        static int status = 0;
                        status = gpio_get_level(RELAY_2);
                        ESP_LOGI(RX_ESIM_TAG, "\n-------------TRANG THAI LED %d -------------------", status);
                    }
                    else
                    {
                        printf("-----------OFF RELAY %d-----------\r\n", data[i + 31]);
                        gpio_set_level(GPIO_LOAD_PIN[num_load - 1], 0);
                    }
                }
                // printf("data[%d]: %c\r\n", i, data[i]);
            }
        }
        // vTaskDelay(2 / portTICK_PERIOD_MS);
    }
    free(data);
}
static void update_status(void *arg)
{
    while (1)
    {
        // static const char *UPLOAD_STATUS = "UPLOAD_STATUS";
        // esp_log_level_set(UPLOAD_STATUS, ESP_LOG_INFO);
        // static int status = 1;
        // status = gpio_get_level(RELAY_2);
        // ESP_LOGI(UPLOAD_STATUS, "\n-------------TRANG THAI LED %d -------------------", status);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
static void open_simcom(void)
{
    gpio_set_level(A7672_PWRKEY, 1);
    ESP_LOGI(TAG, "-----------------kich hoat module sim------------------");
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(A7672_PWRKEY, 0);
    sendData(TAG, "AT+CRESET\r\n");
    // ESP_LOGI(TAG, "-----------------kich hoat module sim------------------");
    // vTaskDelay(pdMS_TO_TICKS(500));
}
void esim_config(void)
{
    init_uart();
    configure_output(A7672_PWRKEY);
    configure_output(RELAY_1);
    configure_output(RELAY_2);
    configure_output(RELAY_3);
    configure_output(RELAY_4);
    open_simcom();
    xTaskCreate(rx_esim, "read_esim", 2048, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(tx_esim, "write_esim", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(update_status, "update_status_load", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
}