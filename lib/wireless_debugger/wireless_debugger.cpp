#include <Arduino.h>
#include <ArduinoLog.h>

#include "wireless_debugger.h"

static wireless_debugger_params_t wireless_debugger_params = {0};

void wireless_debugger_init(void)
{
    wireless_debugger_off();
}

void wireless_debugger_on(void)
{
    Log.noticeln("Wireless debugger turned ON.");
    wireless_debugger_params.is_debugger_on = true;
}

void wireless_debugger_off(void)
{
    Log.noticeln("Wireless debugger turned OFF.");
    wireless_debugger_params.is_debugger_on = false;
}

bool is_wireless_debugger_on(void)
{
    return wireless_debugger_params.is_debugger_on;
}


/* 
--------------------------------------------------
    GET FUNCTIONS
--------------------------------------------------
*/
// -------- BYTE 0 --------

bool get_wireless_debugger_is_spa_configured(void)
{
    return (bool) wireless_debugger_params.status.bits.is_spa_configured;
}
bool get_wireless_debugger_i2c_read_status(void)
{
    return (bool) wireless_debugger_params.status.bits.i2c_read_status;
}
bool get_wireless_debugger_i2c_write_status(void)
{
    return (bool) wireless_debugger_params.status.bits.i2c_write_status;
}
uint8_t get_wireless_debugger_i2c_master_slave_status(I2C_ROLE_e role_)
{
    return (uint8_t) wireless_debugger_params.status.bits.i2c_master_slave_status;
}
uint8_t get_wireless_debugger_spa_configuration(void)
{
    return wireless_debugger_params.status.bytes[0];
}


// -------- BYTE 1 --------

uint16_t get_wireless_debugger_i2c_failed_counter(void)
{
    return (uint16_t) wireless_debugger_params.status.bits.i2c_failed_counter;
}


// -------- BYTE 3 --------

uint16_t get_wireless_debugger_wifi_reconn_counter(void)
{
    return (uint16_t) wireless_debugger_params.status.bits.wifi_reconn_counter;
}


// -------- BYTE 5 --------

uint16_t get_wireless_debugger_mqtt_reconn_counter(void)
{
    return (uint16_t) wireless_debugger_params.status.bits.mqtt_reconn_counter;
}


// -------- BYTE 7 --------

bool get_wireless_debugger_pump1_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.pump1_set_cmd_status;
}
bool get_wireless_debugger_pump2_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.pump2_set_cmd_status;
}
bool get_wireless_debugger_pump3_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.pump3_set_cmd_status;
}
bool get_wireless_debugger_pump4_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.pump4_set_cmd_status;
}
bool get_wireless_debugger_pump5_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.pump5_set_cmd_status;
}
bool get_wireless_debugger_pump6_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.pump1_set_cmd_status;
}
bool get_wireless_debugger_light1_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.light1_set_cmd_status;
}
bool get_wireless_debugger_light2_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.light1_set_cmd_status;
}


// -------- BYTE 8 --------

bool get_wireless_debugger_blower_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.blower_set_cmd_status;
}
bool get_wireless_debugger_waterfall_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.waterfall_set_cmd_status;
}
bool get_wireless_debugger_tempScale_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.tempScale_set_cmd_status;
}
bool get_wireless_debugger_time_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.time_set_cmd_status;
}
bool get_wireless_debugger_setTemp_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.setTemp_set_cmd_status;
}
bool get_wireless_debugger_filterCycle_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.filterCycle_set_cmd_status;
}
bool get_wireless_debugger_ecoCycle_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.ecoCycle_set_cmd_status;
}
bool get_wireless_debugger_resetWifiSta_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.resetWifiSta_set_cmd_status;
}


// -------- BYTE 9 --------

bool get_wireless_debugger_errorCodeURL_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.errorCodeURL_set_cmd_status;
}
bool get_wireless_debugger_keypadState_set_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.keypadState_set_cmd_status;
}


// -------- BYTE 11 --------

bool get_wireless_debugger_deviceStatus_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.deviceStatus_get_cmd_status;
}
bool get_wireless_debugger_tempScale_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.tempScale_get_cmd_status;
}
bool get_wireless_debugger_time_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.time_get_cmd_status;
}
bool get_wireless_debugger_setTemp_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.setTemp_get_cmd_status;
}
bool get_wireless_debugger_currentTemp_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.currentTemp_get_cmd_status;
}
bool get_wireless_debugger_filterCycle_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.filterCycle_get_cmd_status;
}
bool get_wireless_debugger_ecoCycle_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.ecoCycle_get_cmd_status;
}
bool get_wireless_debugger_setupInfo_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.setupInfo_get_cmd_status;
}


// -------- BYTE 12 --------

bool get_wireless_debugger_fwVersion_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.fwVersion_get_cmd_status;
}
bool get_wireless_debugger_keypadState_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.keypadState_get_cmd_status;
}
bool get_wireless_debugger_heatMode_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.heatMode_get_cmd_status;
}
bool get_wireless_debugger_tempRange_get_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.tempRange_get_cmd_status;
}


// -------- BYTE 15 --------

bool get_wireless_debugger_ota_cmd_status(void)
{
    return (bool) wireless_debugger_params.status.bits.ota_cmd_status;
}

// -------- ALL BYTES --------
size_t get_wireless_debugger_debug_status_bytes(uint8_t *__buff)
{
    memcpy(&__buff[0], &wireless_debugger_params.status.bytes[0], sizeof(wireless_debugger_params.status.bytes));
    return sizeof(wireless_debugger_params.status.bytes);
}


/* 
--------------------------------------------------
    SET FUNCTIONS
--------------------------------------------------
*/
// -------- BYTE 0 --------

void set_wireless_debugger_is_spa_configured(bool status_)
{
    wireless_debugger_params.status.bits.is_spa_configured = status_ ? 1 : 0;
}
void set_wireless_debugger_i2c_read_status(bool status_)
{
    wireless_debugger_params.status.bits.i2c_read_status = status_ ? 1 : 0;
}

void set_wireless_debugger_i2c_write_status(bool status_)
{
    wireless_debugger_params.status.bits.i2c_write_status = status_ ? 1 : 0;
}

void set_wireless_debugger_i2c_master_slave_status(I2C_ROLE_e role_)
{
    wireless_debugger_params.status.bits.i2c_master_slave_status = (uint8_t) role_;
}


// -------- BYTE 1 --------

void set_wireless_debugger_i2c_failed_counter_up(void)
{
    if (wireless_debugger_params.status.bits.i2c_failed_counter >= 65535U) {
        wireless_debugger_params.status.bits.i2c_failed_counter = 0;
        return;
    }
    wireless_debugger_params.status.bits.i2c_failed_counter++;
}


// -------- BYTE 3 --------

void set_wireless_debugger_wifi_reconn_counter_up(void)
{
    if (wireless_debugger_params.status.bits.wifi_reconn_counter >= 65535U) {
        wireless_debugger_params.status.bits.wifi_reconn_counter = 0;
        return;
    }
    wireless_debugger_params.status.bits.wifi_reconn_counter++;
}


// -------- BYTE 5 --------

void set_wireless_debugger_mqtt_reconn_counter_up(void)
{
    if (wireless_debugger_params.status.bits.mqtt_reconn_counter >= 65535U) {
        wireless_debugger_params.status.bits.mqtt_reconn_counter = 0;
        return;
    }
    wireless_debugger_params.status.bits.mqtt_reconn_counter++;
}


// -------- BYTE 7 --------

void set_wireless_debugger_pump1_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.pump1_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_pump2_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.pump2_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_pump3_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.pump3_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_pump4_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.pump4_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_pump5_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.pump5_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_pump6_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.pump6_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_light1_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.light1_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_light2_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.light2_set_cmd_status = status_ ? 1 : 0;
}


// -------- BYTE 8 --------

void set_wireless_debugger_blower_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.blower_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_waterfall_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.waterfall_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_tempScale_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.tempScale_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_time_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.time_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_setTemp_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.setTemp_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_filterCycle_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.filterCycle_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_ecoCycle_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.ecoCycle_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_resetWifiSta_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.resetWifiSta_set_cmd_status = status_ ? 1 : 0;
}


// -------- BYTE 9 --------

void set_wireless_debugger_errorCodeURL_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.errorCodeURL_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_keypadState_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.keypadState_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_heatMode_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.heatMode_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_tempRange_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.tempRange_set_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_clockMode_set_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.clockMode_set_cmd_status = status_ ? 1 : 0;
}


// -------- BYTE 11 --------

void set_wireless_debugger_deviceStatus_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.deviceStatus_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_tempScale_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.tempScale_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_time_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.time_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_setTemp_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.setTemp_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_currentTemp_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.currentTemp_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_filterCycle_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.filterCycle_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_ecoCycle_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.ecoCycle_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_setupInfo_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.setupInfo_get_cmd_status = status_ ? 1 : 0;
}


// -------- BYTE 12 --------

void set_wireless_debugger_fwVersion_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.fwVersion_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_keypadState_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.keypadState_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_heatMode_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.heatMode_get_cmd_status = status_ ? 1 : 0;
}
void set_wireless_debugger_tempRange_get_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.tempRange_get_cmd_status = status_ ? 1 : 0;
}


// -------- BYTE 15 --------

void set_wireless_debugger_ota_cmd_status(bool status_)
{
    wireless_debugger_params.status.bits.ota_cmd_status = status_ ? 1 : 0;
}


/* 
--------------------------------------------------
    RESET FUNCTIONS
--------------------------------------------------
*/
// -------- BYTE 1 --------

void set_wireless_debugger_i2c_failed_counter_reset(void)
{
    wireless_debugger_params.status.bits.i2c_failed_counter = 0;
}


// -------- BYTE 3 --------

void set_wireless_debugger_wifi_reconn_counter_reset(void)
{
    wireless_debugger_params.status.bits.wifi_reconn_counter = 0;
}


// -------- BYTE 5 --------

void set_wireless_debugger_mqtt_reconn_counter_reset(void)
{
    wireless_debugger_params.status.bits.mqtt_reconn_counter = 0;
}