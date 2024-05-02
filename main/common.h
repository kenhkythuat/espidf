#ifndef _COMMON_H_
#define _COMMON_H_
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "cJSON.h"

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

#define MQTT_TOPIC_ACTUATOR_STATUS FARM "/sn/" SERIAL_NUMBER "/as/"
// MQTT topic to subscribe and get command to switch on/off actuator
#define MQTT_TOPIC_ACTUATOR_CONTROL FARM "/snac/" SERIAL_NUMBER "/"

#define NUMBER_LOADS 4
#define LENGTH_STATUS_PAYLOAD_0_9 6 * NUMBER_LOADS + 1
#define LENGTH_STATUS_PAYLOAD_10_18 6 * NUMBER_LOADS + 2 + NUMBER_LOADS % 10

#define TXD_PIN (GPIO_NUM_1)
#define RXD_PIN (GPIO_NUM_2)
#define A7672_PWRKEY (GPIO_NUM_42)
#define RELAY_1 (GPIO_NUM_10)
#define RELAY_2 (GPIO_NUM_9)
#define RELAY_3 (GPIO_NUM_46)
#define RELAY_4 (GPIO_NUM_3)
#define BLINK_LED (GPIO_NUM_4)
#define ON_OFF_24V (GPIO_NUM_13)

void esim_config(void);
void config_pwm_50hz(void);
// static void update_status(void *arg);
// static void rx_esim(void *arg);
// static void tx_esim(void *arg)

#endif