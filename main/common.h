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

#define MQTT_TOPIC_ACTUATOR_STATUS FARM "/sn/" SERIAL_NUMBER "/as/"
// MQTT topic to subscribe and get command to switch on/off actuator
#define MQTT_TOPIC_ACTUATOR_CONTROL FARM "/snac/" SERIAL_NUMBER "/"

#define NUMBER_LOADS 4
#define LENGTH_STATUS_PAYLOAD_0_9 6 * NUMBER_LOADS + 1
#define LENGTH_STATUS_PAYLOAD_10_18 6 * NUMBER_LOADS + 2 + NUMBER_LOADS % 10

void esim_config(void);

#endif