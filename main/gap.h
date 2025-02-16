#ifndef GAP_H
#define GAP_H

#pragma once
#include "esp_gap_ble_api.h"

extern uint8_t adv_config_done;
extern uint8_t adv_service_uuid128[32];
extern esp_ble_adv_data_t adv_data;
extern esp_ble_adv_data_t scan_rsp_data;
extern esp_ble_adv_params_t adv_params;

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void gap_init(void);


#endif // GAP_H
