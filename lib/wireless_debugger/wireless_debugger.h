#include <stdint.h>
#include <stdbool.h>

#define WIRELESS_DEBUGGER_MQTT_PUB_TOPIC_PREFFIX    "debugger"


typedef enum {
    I2C_SLAVE,
    I2C_MASTER
} I2C_ROLE_e;


typedef union
{
    struct
    {
        // -------- BYTE 0 --------
        uint8_t is_spa_configured           : 1;
        uint8_t i2c_master_slave_status     : 1;
        uint8_t i2c_read_status             : 1;
        uint8_t i2c_write_status            : 1;
        uint8_t reserved1                   : 4;
        
        // -------- BYTE 1 --------
        uint16_t i2c_failed_counter          : 16;   // fill byte
        
        // -------- BYTE 3 --------
        uint16_t wifi_reconn_counter         : 16;   // fill byte
        
        // -------- BYTE 5 --------
        uint16_t mqtt_reconn_counter         : 16;   // fill byte

        // -------- BYTE 7 --------
        uint8_t pump1_set_cmd_status                : 1;
        uint8_t pump2_set_cmd_status                : 1;
        uint8_t pump3_set_cmd_status                : 1;
        uint8_t pump4_set_cmd_status                : 1;
        uint8_t pump5_set_cmd_status                : 1;
        uint8_t pump6_set_cmd_status                : 1;
        uint8_t light1_set_cmd_status               : 1;
        uint8_t light2_set_cmd_status               : 1;

        // -------- BYTE 8 --------
        uint8_t blower_set_cmd_status               : 1;
        uint8_t waterfall_set_cmd_status            : 1;
        uint8_t tempScale_set_cmd_status            : 1;
        uint8_t time_set_cmd_status                 : 1;
        uint8_t setTemp_set_cmd_status              : 1;
        uint8_t filterCycle_set_cmd_status          : 1;
        uint8_t ecoCycle_set_cmd_status             : 1;
        uint8_t resetWifiSta_set_cmd_status         : 1;
        
        // -------- BYTE 9 --------
        uint8_t errorCodeURL_set_cmd_status         : 1;
        uint8_t keypadState_set_cmd_status          : 1;
        uint8_t heatMode_set_cmd_status             : 1;
        uint8_t tempRange_set_cmd_status            : 1;
        uint8_t clockMode_set_cmd_status            : 1;
        uint8_t reserved2                           : 3;
        
        // -------- BYTE 10 --------
        uint8_t reserved3                           : 8;
        
        // -------- BYTE 11 --------
        uint8_t deviceStatus_get_cmd_status         : 1;
        uint8_t tempScale_get_cmd_status            : 1;
        uint8_t time_get_cmd_status                 : 1;
        uint8_t setTemp_get_cmd_status              : 1;
        uint8_t currentTemp_get_cmd_status          : 1;
        uint8_t filterCycle_get_cmd_status          : 1;
        uint8_t ecoCycle_get_cmd_status             : 1;
        uint8_t setupInfo_get_cmd_status            : 1;
        
        // -------- BYTE 12 --------
        uint8_t fwVersion_get_cmd_status            : 1;
        uint8_t keypadState_get_cmd_status          : 1;
        uint8_t heatMode_get_cmd_status             : 1;
        uint8_t tempRange_get_cmd_status            : 1;
        uint8_t reserved4                           : 4;
        
        // -------- BYTE 13 --------
        uint8_t reserved5                           : 8;
        uint8_t reserved6                           : 8;
        
        // -------- BYTE 15 --------
        uint8_t ota_cmd_status                      : 1;
        uint8_t reserved7                           : 7;
        uint8_t reserved8                           : 8;


    } __attribute__((packed)) bits;

    uint8_t bytes[17];
} debug_status_t;

typedef union {
    bool                is_debugger_on;
    debug_status_t      status;
} wireless_debugger_params_t;

void wireless_debugger_init(void);

void wireless_debugger_on(void);

void wireless_debugger_off(void);

bool is_wireless_debugger_on(void);


/* ============= GET =================== */
// -------- BYTE 0 --------
bool get_wireless_debugger_is_spa_configured(void);
bool get_wireless_debugger_i2c_read_status(void);
bool get_wireless_debugger_i2c_write_status(void);
uint8_t get_wireless_debugger_i2c_master_slave_status(I2C_ROLE_e role_);
uint8_t get_wireless_debugger_spa_configuration(void);

// -------- BYTE 1 --------
uint16_t get_wireless_debugger_i2c_failed_counter(void);

// -------- BYTE 3 --------
uint16_t get_wireless_debugger_wifi_reconn_counter(void);

// -------- BYTE 5 --------
uint16_t get_wireless_debugger_mqtt_reconn_counter(void);

// -------- BYTE 7 --------
bool get_wireless_debugger_pump1_cmd_status(void);
bool get_wireless_debugger_pump2_cmd_status(void);
bool get_wireless_debugger_pump3_cmd_status(void);
bool get_wireless_debugger_pump4_cmd_status(void);
bool get_wireless_debugger_pump5_cmd_status(void);
bool get_wireless_debugger_pump6_cmd_status(void);
bool get_wireless_debugger_light1_cmd_status(void);
bool get_wireless_debugger_light2_cmd_status(void);

// -------- BYTE 8 --------
bool get_wireless_debugger_blower_cmd_status(void);
bool get_wireless_debugger_waterfall_cmd_status(void);
bool get_wireless_debugger_tempScale_set_cmd_status(void);
bool get_wireless_debugger_time_set_cmd_status(void);
bool get_wireless_debugger_setTemp_set_cmd_status(void);
bool get_wireless_debugger_filterCycle_set_cmd_status(void);
bool get_wireless_debugger_ecoCycle_set_cmd_status(void);
bool get_wireless_debugger_resetWifiSta_set_cmd_status(void);

// -------- BYTE 9 --------
bool get_wireless_debugger_errorCodeURL_set_cmd_status(void);
bool get_wireless_debugger_keypadState_set_cmd_status(void);

// -------- BYTE 11 --------
bool get_wireless_debugger_deviceStatus_get_cmd_status(void);
bool get_wireless_debugger_tempScale_get_cmd_status(void);
bool get_wireless_debugger_time_get_cmd_status(void);
bool get_wireless_debugger_setTemp_get_cmd_status(void);
bool get_wireless_debugger_currentTemp_get_cmd_status(void);
bool get_wireless_debugger_filterCycle_get_cmd_status(void);
bool get_wireless_debugger_ecoCycle_get_cmd_status(void);
bool get_wireless_debugger_setupInfo_get_cmd_status(void);

// -------- BYTE 12 --------
bool get_wireless_debugger_fwVersion_get_cmd_status(void);
bool get_wireless_debugger_keypadState_get_cmd_status(void);
bool get_wireless_debugger_heatMode_get_cmd_status(void);
bool get_wireless_debugger_tempRange_get_cmd_status(void);

// -------- BYTE 15 --------
bool get_wireless_debugger_ota_cmd_status(void);

// -------- ALL BYTES --------
size_t get_wireless_debugger_debug_status_bytes(uint8_t *__buff);


/* SET */
// -------- BYTE 0 --------
void set_wireless_debugger_is_spa_configured(bool status_);
void set_wireless_debugger_i2c_read_status(bool status_);
void set_wireless_debugger_i2c_write_status(bool status_);
void set_wireless_debugger_i2c_master_slave_status(I2C_ROLE_e role_);

// -------- BYTE 1 --------
void set_wireless_debugger_i2c_failed_counter_up(void);

// -------- BYTE 3 --------
void set_wireless_debugger_wifi_reconn_counter_up(void);

// -------- BYTE 5 --------
void set_wireless_debugger_mqtt_reconn_counter_up(void);

// -------- BYTE 7 --------
void set_wireless_debugger_pump1_cmd_status(bool status_);
void set_wireless_debugger_pump2_cmd_status(bool status_);
void set_wireless_debugger_pump3_cmd_status(bool status_);
void set_wireless_debugger_pump4_cmd_status(bool status_);
void set_wireless_debugger_pump5_cmd_status(bool status_);
void set_wireless_debugger_pump6_cmd_status(bool status_);
void set_wireless_debugger_light1_cmd_status(bool status_);
void set_wireless_debugger_light2_cmd_status(bool status_);

// -------- BYTE 8 --------
void set_wireless_debugger_blower_cmd_status(bool status_);
void set_wireless_debugger_waterfall_cmd_status(bool status_);
void set_wireless_debugger_tempScale_set_cmd_status(bool status_);
void set_wireless_debugger_time_set_cmd_status(bool status_);
void set_wireless_debugger_setTemp_set_cmd_status(bool status_);
void set_wireless_debugger_filterCycle_set_cmd_status(bool status_);
void set_wireless_debugger_ecoCycle_set_cmd_status(bool status_);
void set_wireless_debugger_resetWifiSta_set_cmd_status(bool status_);

// -------- BYTE 9 --------
void set_wireless_debugger_errorCodeURL_set_cmd_status(bool status_);
void set_wireless_debugger_keypadState_set_cmd_status(bool status_);
void set_wireless_debugger_heatMode_set_cmd_status(bool status_);
void set_wireless_debugger_tempRange_set_cmd_status(bool status_);
void set_wireless_debugger_clockMode_set_cmd_status(bool status_);

// -------- BYTE 11 --------
void set_wireless_debugger_deviceStatus_get_cmd_status(bool status_);
void set_wireless_debugger_tempScale_get_cmd_status(bool status_);
void set_wireless_debugger_time_get_cmd_status(bool status_);
void set_wireless_debugger_setTemp_get_cmd_status(bool status_);
void set_wireless_debugger_currentTemp_get_cmd_status(bool status_);
void set_wireless_debugger_filterCycle_get_cmd_status(bool status_);
void set_wireless_debugger_ecoCycle_get_cmd_status(bool status_);
void set_wireless_debugger_setupInfo_get_cmd_status(bool status_);

// -------- BYTE 12 --------
void set_wireless_debugger_fwVersion_get_cmd_status(bool status_);
void set_wireless_debugger_keypadState_get_cmd_status(bool status_);
void set_wireless_debugger_heatMode_get_cmd_status(bool status_);
void set_wireless_debugger_tempRange_get_cmd_status(bool status_);

// -------- BYTE 15 --------
void set_wireless_debugger_ota_cmd_status(bool status_);

/* RESET */
// -------- BYTE 1 --------
void set_wireless_debugger_i2c_failed_counter_reset(void);

// -------- BYTE 3 --------
void set_wireless_debugger_wifi_reconn_counter_reset(void);

// -------- BYTE 5 --------
void set_wireless_debugger_mqtt_reconn_counter_reset(void);

