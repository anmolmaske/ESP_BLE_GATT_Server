#ifndef GATT_SERVER_H
#define GATT_SERVER_H

#include "esp_gatts_api.h"

// Number of GATT profiles
#define PROFILE_NUM 2
#define PROFILE_A_APP_ID 0
#define PROFILE_B_APP_ID 1

// Maximum size for prepared write buffer
#define PREPARE_BUF_MAX_SIZE 1024

// Structure to manage prepared write operations
typedef struct {
    uint8_t *prepare_buf; // Pointer to the buffer for prepared writes
    int prepare_len;      // Length of the prepared write data
} prepare_type_env_t;

// Structure to store GATT server profile information
struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;      // GATT event callback function
    uint16_t gatts_if;            // GATT interface handle
    uint16_t app_id;              // Application ID
    uint16_t conn_id;             // Connection ID
    uint16_t service_handle;      // Handle for the service
    esp_gatt_srvc_id_t service_id; // Service UUID and other details
    uint16_t char_handle;         // Handle for the characteristic
    esp_bt_uuid_t char_uuid;      // UUID of the characteristic
    esp_gatt_perm_t perm;         // Permission settings
    esp_gatt_char_prop_t property; // Characteristic properties (Read, Write, Notify, etc.)
    uint16_t descr_handle;        // Handle for the descriptor
    esp_bt_uuid_t descr_uuid;     // UUID of the descriptor
};

// Global profile instance array for GATT server
extern struct gatts_profile_inst gl_profile_tab[PROFILE_NUM];

// Function prototypes
void gatt_server_init(void); // Initialize the GATT server
void example_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

#endif // GATT_SERVER_H
