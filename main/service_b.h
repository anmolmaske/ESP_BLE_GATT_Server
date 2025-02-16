#ifndef SERVICE_B_H
#define SERVICE_B_H

#pragma once

#include "esp_gatts_api.h"

#define GATTS_SERVICE_UUID_TEST_B 0x00EE /* Service UUID */
#define GATTS_CHAR_UUID_TEST_B 0xEE01 /* Characteristic UUID */
#define GATTS_DESCR_UUID_TEST_B 0x2222 /* Descriptor UUID */
#define GATTS_NUM_HANDLE_TEST_B 4 /* Number of handles */

/* Function prototype for Profile B event handler */
void profile_b_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

#endif /* SERVICE_B_H */
