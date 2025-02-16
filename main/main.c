#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"

#include "nvs_flash.h"
#include "esp_log.h"

#include "gap.h"
#include "gatt_server.h"

#define TAG "MAIN" // Logging tag for debugging


// Function to get and print BLE MAC Address
void print_ble_mac_address(void) {
    const uint8_t *ble_mac = esp_bt_dev_get_address();
    
    if (ble_mac) {
        char ble_mac_str[18];  // "XX:XX:XX:XX:XX:XX" + null terminator

        // Format MAC address into a string
        snprintf(ble_mac_str, sizeof(ble_mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                 ble_mac[0], ble_mac[1], ble_mac[2], ble_mac[3], ble_mac[4], ble_mac[5]);

        // Print the MAC address
        ESP_LOGI(TAG, "ESP32 BLE MAC Address: %s\n", ble_mac_str);
    } else {
        ESP_LOGI(TAG, "Failed to retrieve BLE MAC Address\n");
    }
}


void app_main(void) {
    esp_err_t ret;

    // Initialize Non-Volatile Storage (NVS)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase()); // Erase NVS if required
        ret = nvs_flash_init(); // Reinitialize NVS
    }
    ESP_ERROR_CHECK(ret); // Check for errors

    // Release memory allocated for Classic Bluetooth since we only use BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // Configure and initialize the Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // Enable the Bluetooth controller in BLE mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // Initialize the Bluedroid Bluetooth stack
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // Enable Bluedroid
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // Get the BLE MAC Address
    print_ble_mac_address();

    // Initialize the GATT server
    gatt_server_init();

    // Initialize the GAP (Generic Access Profile) layer
    gap_init();
}
