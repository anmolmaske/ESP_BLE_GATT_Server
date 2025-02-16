#include "esp_gatt_common_api.h"
#include "esp_gatt_defs.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "gatt_server.h"
#include "service_a.h"
#include "service_b.h"

#define GATTS_TAG "GATT_SERVER"

/* One GATT-based profile per app_id and gatts_if. 
 * This array stores the gatts_if returned by ESP_GATTS_REG_EVT.
 */
struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gatts_cb = profile_a_event_handler,  // Callback function for Profile A events
        .gatts_if = ESP_GATT_IF_NONE,         // Initially set to ESP_GATT_IF_NONE (not yet assigned)
    },
    [PROFILE_B_APP_ID] = {
        .gatts_cb = profile_b_event_handler,  // Callback function for Profile B events
        .gatts_if = ESP_GATT_IF_NONE,         // Initially set to ESP_GATT_IF_NONE (not yet assigned)
    },
};

/* Event handler for GATT server events */
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            // Store the gatts_if for the corresponding application ID
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGE(GATTS_TAG, "Reg app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
            return;
        }
    }

    /* If gatts_if is ESP_GATT_IF_NONE, call all profile callbacks.
     * Otherwise, call the specific profile callback.
     */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE ||  // Call all profiles if not specific
                gatts_if == gl_profile_tab[idx].gatts_if) {
                if (gl_profile_tab[idx].gatts_cb) {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

/* Initializes the GATT server */
void gatt_server_init(void) {
    esp_ble_gatts_register_callback(gatts_event_handler);  // Register the main event handler
    esp_ble_gatts_app_register(PROFILE_A_APP_ID);          // Register Profile A
    esp_ble_gatts_app_register(PROFILE_B_APP_ID);          // Register Profile B
    esp_ble_gatt_set_local_mtu(500);                       // Set the MTU size to 500 bytes
}

/* Handles write events for characteristics with prepare write enabled */
void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    esp_gatt_status_t status = ESP_GATT_OK;
    
    if (param->write.need_rsp) { // Check if a response is needed
        if (param->write.is_prep) { // If it's a prepare write operation
            // Validate offset and length
            if (param->write.offset > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_OFFSET;
            } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                status = ESP_GATT_INVALID_ATTR_LEN;
            }
            
            // Allocate memory for the prepare write buffer if not already allocated
            if (status == ESP_GATT_OK && prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(GATTS_TAG, "Gatt_server prep no mem");
                    status = ESP_GATT_NO_RESOURCES;
                }
            }

            // Allocate and send a response to the client
            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            if (gatt_rsp) {
                gatt_rsp->attr_value.len = param->write.len;
                gatt_rsp->attr_value.handle = param->write.handle;
                gatt_rsp->attr_value.offset = param->write.offset;
                gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
                memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
                
                esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
                if (response_err != ESP_OK) {
                    ESP_LOGE(GATTS_TAG, "Send response error\n");
                }
                free(gatt_rsp);
            } else {
                ESP_LOGE(GATTS_TAG, "malloc failed, no resource to send response error\n");
                status = ESP_GATT_NO_RESOURCES;
            }

            // Return if an error occurred
            if (status != ESP_GATT_OK) {
                return;
            }

            // Copy received data into the prepare buffer
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;

        } else { // Handle normal write request (not prepare write)
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
        }
    }
}

/* Handles execute write event for prepared writes */
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC) {
        // Log the received data as a hex buffer
        esp_log_buffer_hex(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    } else {
        ESP_LOGI(GATTS_TAG, "ESP_GATT_PREP_WRITE_CANCEL");
    }

    // Free allocated buffer memory after execution
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}
