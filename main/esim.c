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
#define TAG "----MQTT Esim----"
char AT_RESET[] = "AT+CRESET\r\n";
char AT_CHECK_A76XX[] = "AT\r\n";
char AT_CHECK_ESIM[] = "AT+CGREG?\r\n";
char AT_START_MQTT[] = "AT+CMQTTSTART\r\n";
char AT_ACQUIRE_CLIENT[] = "AT+CMQTTACCQ=0,\"%s\",0\r\n";
char AT_CONNECT_MQTT[] = "AT+CMQTTCONNECT=0,\"%s:%d\",60,1,\"%s\",\"%s\"\r\n";
char AT_SET_PUBLISH_TOPIC[] = "AT+CMQTTTOPIC=0,%d\r\n";
char AT_SET_PUBLISH_PAYLOAD[] = "AT+CMQTTPAYLOAD=0,%d\r\n";
char AT_PUBLISH[] = "AT+CMQTTPUB=0,1,60\r\n";
char AT_SUBCRIBE[] = "AT+CMQTTSUB=0\r\n";
char AT_SET_SUBCRIBE_0_9_TOPIC[] = "AT+CMQTTSUBTOPIC=0,35,1\r\n";
char AT_SET_SUBCRIBE_10_18_TOPIC[] = "AT+CMQTTSUBTOPIC=0,36,1\r\n";
char AT_SLEEP_MODE2[] = "AT+CSCLK=2\r\n";
char AT_SUBCRIBE_TOPIC[] = "%s%d\r\n";
char AT_COMMAND[100];
char AT_INFORM_PAYLOAD[] = "{%d:%d}\r\n";
char AT_COMMAND[100];
char STATUS_PAYLOAD_ARRAY_0_9[LENGTH_STATUS_PAYLOAD_0_9];
char STATUS_PAYLOAD_ARRAY_10_18[LENGTH_STATUS_PAYLOAD_10_18];
char STATUS_PAYLOAD_ARRAY_TOTAL[] = "{\"1\":0,\"2\":0,\"3\":0,\"4\":0,\"5\":0,\"6\":0,\"7\":0,\"8\":0,\"9\":0,\"10\":0,\"11\":0,\"12\":0,\"13\":0,\"14\":0,\"15\":0,\"16\":0,\"17\":0,\"18\":0}";
int statusOfLoad;
bool isConnectedMQTT = false;
bool isQueueRx = false;
unsigned int GPIO_LOAD_PIN[10] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4};
int onReay = 0;

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
    gpio_reset_pin(num_gpio);
    gpio_set_direction(num_gpio, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level(num_gpio, 0);
}

static void tx_esim(void *arg)
{
    static const char *TX_TASK_TAG = "---WRITE ESIM---";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1)
    {
        if (isPBDONE == true)
        {
            sendData(TX_TASK_TAG, AT_CHECK_A76XX);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, "ATE0\r\n");
            vTaskDelay(200 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, "AT+CSQ\r\n");
            vTaskDelay(200 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, "AT+CMQTTSTOP\r\n");
            vTaskDelay(200 / portTICK_PERIOD_MS);
            sendData(TX_TASK_TAG, AT_START_MQTT);
            vTaskDelay(200 / portTICK_PERIOD_MS);

            sprintf(AT_COMMAND, AT_ACQUIRE_CLIENT, MQTT_CLIENT_ID);
            sendData(TX_TASK_TAG, AT_COMMAND);
            vTaskDelay(200 / portTICK_PERIOD_MS);

            sprintf(AT_COMMAND, AT_CONNECT_MQTT, MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS);
            sendData(TX_TASK_TAG, AT_COMMAND);
            vTaskDelay(200 / portTICK_PERIOD_MS);

            sprintf(AT_COMMAND, AT_ACQUIRE_CLIENT, MQTT_CLIENT_ID);
            sendData(TX_TASK_TAG, AT_COMMAND);
            vTaskDelay(200 / portTICK_PERIOD_MS);
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
                sendData(TX_TASK_TAG, AT_SUBCRIBE);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            vTaskDelay(200 / portTICK_PERIOD_MS);
            gpio_set_level(BLINK_LED, 1);
            isConnectedMQTT = true;
            // sendData(TX_TASK_TAG, AT_SLEEP_MODE2);
            vTaskSuspend(NULL);
            ESP_LOGI(TAG, "-----------------CREATED ESIM------------------");
        }
    }
}
static void rx_esim(void *arg)
{
    static const char *RX_ESIM_TAG = "RX_TASK_ESIM";
    esp_log_level_set(RX_ESIM_TAG, ESP_LOG_INFO);
    char *data = (char *)malloc(RX_BUF_SIZE + 1);
    while (1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 100 / portTICK_PERIOD_MS);
        if (rxBytes > 0)
        {
            // isQueueRx = true;
            data[rxBytes] = 0;
            ESP_LOGI(RX_ESIM_TAG, "\n-------------Read %d bytes: '%s'-------------------", rxBytes, data);
            if (strstr(data, "PB DONE"))
            {
                isPBDONE = true;
                printf("-----------PB DONE-----------\r\n");
            }
            for (int i = 0; i < rxBytes; i++)
            {
                if ((char)data[i] == '1' && (char)data[i + 1] == '5' && (char)data[i + 2] == '6')
                {
                    static int num_load = 0;
                    static int status;
                    num_load = data[i + 4] - 48;
                    printf("-----------RELAY  %d DA tim thay-----------\r\n", num_load);

                    if (data[i + 29] == 49 && isPBDONE == true)
                    {
                        printf("-----------ON RELAY %d-----------\r\n", data[i + 31]);
                        gpio_set_level(GPIO_LOAD_PIN[num_load - 1], 1);
                        status = gpio_get_level(GPIO_LOAD_PIN[num_load - 1]);
                        ESP_LOGI(RX_ESIM_TAG, "\n-------------TRANG THAI LED %d -------------------", status);
                        onReay++;
                        if (onReay >= NUMBER_LOADS)
                        {
                            onReay = NUMBER_LOADS;
                        }
                        ESP_LOGI(RX_ESIM_TAG, "\n-------------so luong tai bat %d -------------------", onReay);
                    }
                    else if (data[i + 29] == 48 && isPBDONE == true)
                    {
                        printf("-----------OFF RELAY %d-----------\r\n", data[i + 31]);
                        gpio_set_level(GPIO_LOAD_PIN[num_load - 1], 0);
                        status = gpio_get_level(GPIO_LOAD_PIN[num_load - 1]);
                        ESP_LOGI(RX_ESIM_TAG, "\n-------------TRANG THAI LED %d -------------------", status);
                        onReay--;
                        if (onReay < 1)
                        {
                            onReay = 0;
                        }
                        ESP_LOGI(RX_ESIM_TAG, "\n-------------so luong tai tat %d -------------------", onReay);
                    }
                }

                printf("data[%d]: %c\r\n", i, data[i]);
            }
            isQueueRx = false;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    free(data);
}
static void update_status(void *arg)
{
    while (1)
    {
        static const char *UPLOAD_STATUS = "UPLOAD_STATUS";
        esp_log_level_set(UPLOAD_STATUS, ESP_LOG_INFO);
        if (NUMBER_LOADS < 10 && isConnectedMQTT == true && onReay >= 1)
        // if (NUMBER_LOADS < 10 && isConnectedMQTT == true)
        {
            memcpy(STATUS_PAYLOAD_ARRAY_0_9, STATUS_PAYLOAD_ARRAY_TOTAL, LENGTH_STATUS_PAYLOAD_0_9 - 1);
            STATUS_PAYLOAD_ARRAY_0_9[LENGTH_STATUS_PAYLOAD_0_9 - 1] = '}';
            for (int i = 1; i < NUMBER_LOADS + 1; i++)
            {
                statusOfLoad = gpio_get_level(GPIO_LOAD_PIN[i - 1]);
                STATUS_PAYLOAD_ARRAY_0_9[i * 6 - 1] = statusOfLoad + 48;
            }
            sprintf(AT_COMMAND, AT_SET_PUBLISH_TOPIC, strlen(MQTT_TOPIC_ACTUATOR_STATUS));
            sendData(UPLOAD_STATUS, AT_COMMAND);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            sprintf(AT_COMMAND, "%s\r\n", MQTT_TOPIC_ACTUATOR_STATUS);
            sendData(UPLOAD_STATUS, AT_COMMAND);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            int lengthOfInformPayload = strlen(STATUS_PAYLOAD_ARRAY_0_9);
            sprintf(AT_COMMAND, AT_SET_PUBLISH_PAYLOAD, lengthOfInformPayload);
            sendData(UPLOAD_STATUS, AT_COMMAND);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            sendData(UPLOAD_STATUS, STATUS_PAYLOAD_ARRAY_0_9);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            sendData(UPLOAD_STATUS, AT_PUBLISH);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            // sendData(UPLOAD_STATUS, AT_SLEEP_MODE2);
        }
        vTaskDelay(15000 / portTICK_PERIOD_MS);
    }
}
static void open_simcom(void)
{
    ESP_LOGI(TAG, "-----------------Activated Module Sim------------------");
    // sendData(TAG, "AT+CRESET\r\n");
    gpio_set_level(A7672_PWRKEY, 1);
    vTaskDelay(pdMS_TO_TICKS(3200));
    gpio_set_level(A7672_PWRKEY, 0);
    vTaskDelay(pdMS_TO_TICKS(3000));
    gpio_set_level(A7672_PWRKEY, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_set_level(A7672_PWRKEY, 0);
    ESP_LOGI(TAG, "-----------------Running Module Sim ------------------");
}
void esim_config(void)
{
    init_uart();
    configure_output(A7672_PWRKEY);
    configure_output(RELAY_1);
    configure_output(RELAY_2);
    configure_output(RELAY_3);
    configure_output(RELAY_4);
    configure_output(BLINK_LED);
    // configure_output(ON_OFF_24V);
    // gpio_set_level(ON_OFF_24V, 1);
    ESP_LOGI(TAG, "-----------------Esim config------------------");

    open_simcom();
    xTaskCreate(rx_esim, "read_esim", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreatePinnedToCore(tx_esim, "write_esim", 2048, NULL, configMAX_PRIORITIES - 1, NULL, 0);
    xTaskCreate(update_status, "update_status_load", 2048, NULL, configMAX_PRIORITIES - 3, NULL);
}