#ifndef SERVICE_A_H
#define SERVICE_A_H

#pragma once

#include "esp_gatts_api.h"

#define GATTS_SERVICE_UUID_TEST_A   0x00FF    /* Service UUID */
#define GATTS_CHAR_UUID_TEST_A      0xFF01    /* Characteristic UUID */
#define GATTS_DESCR_UUID_TEST_A     0x3333    /* Descriptor UUID */
#define GATTS_NUM_HANDLE_TEST_A     4         /* Number of handles */

/* Function prototype for Profile A event handler */
void profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

#endif /* SERVICE_A_H */
