#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_log.h"

#include "gap.h"

#define GAP_TAG "GAP" // Tag for logging messages

uint8_t adv_config_done = 0; // Advertisement configuration flag

/* Flags to track advertisement data configuration completion */
#define adv_config_flag (1 << 0) 
#define scan_rsp_config_flag (1 << 1)

/* UUIDs for advertisement services */
uint8_t adv_service_uuid128[32] = {
    /* LSB <-----------------------------------------> MSB */
    // First UUID (16-bit), [12],[13] represent the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    // Second UUID (32-bit), [12], [13], [14], [15] represent the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

/* Advertisement data configuration */
esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false, // Not a scan response packet
    .include_name = true, // Include device name in advertisement
    .include_txpower = false, // Do not include TX power level
    .min_interval = 0x0006, // Min connection interval (7.5ms)
    .max_interval = 0x0010, // Max connection interval (20ms)
    .appearance = 0x00,
    .manufacturer_len = 0, // No manufacturer data
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128), // Include service UUID
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT), // General discovery mode, BR/EDR not supported
};

/* Scan response data configuration */
esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true, // This is a scan response packet
    .include_name = true, // Include device name in scan response
    .include_txpower = true, // Include TX power level
    .appearance = 0x00,
    .manufacturer_len = 0, // No manufacturer data
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128), // Include service UUID
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

/* Advertisement parameters */
esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20, // Minimum advertising interval
    .adv_int_max = 0x40, // Maximum advertising interval
    .adv_type = ADV_TYPE_IND, // Advertising type (connectable, undirected)
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC, // Public Bluetooth address
    .channel_map = ADV_CHNL_ALL, // Advertise on all channels
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // Allow any device to scan and connect
};

/* GAP event handler */
void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        // Advertisement data successfully set
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;

    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        // Scan response data successfully set
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // Advertising start event (success or failure)
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GAP_TAG, "Advertising start failed");
        }
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        // Advertising stop event (success or failure)
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GAP_TAG, "Advertising stop failed");
        } else {
            ESP_LOGI(GAP_TAG, "Stop adv successfully");
        }
        break;

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        // Connection parameters updated
        ESP_LOGI(GAP_TAG, "Update connection params: status = %d, min_int = %d, max_int = %d, conn_int = %d, latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;

    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
        // Packet length update event
        ESP_LOGI(GAP_TAG, "Packet length updated: rx = %d, tx = %d, status = %d",
                  param->pkt_data_length_cmpl.params.rx_len,
                  param->pkt_data_length_cmpl.params.tx_len,
                  param->pkt_data_length_cmpl.status);
        break;

    default:
        break;
    }
}

/* Initialize GAP (Generic Access Profile) */
void gap_init(void) {
    esp_ble_gap_register_callback(gap_event_handler); // Register GAP event handler
    esp_ble_gap_config_adv_data(&adv_data); // Configure advertisement data
    adv_config_done |= adv_config_flag;
    esp_ble_gap_config_adv_data(&scan_rsp_data); // Configure scan response data
    adv_config_done |= scan_rsp_config_flag;
}
