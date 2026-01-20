# Wireless Debugger - Bitwise Structure Documentation

## Overview

This module provides a wireless debugging interface for a spa controller system. It tracks I2C communication status, network connectivity, and command execution states using a 17-byte packed data structure.

## Data Structure Summary

- **Total Size**: 17 bytes (136 bits)
- **Storage Method**: Union type allowing access as either individual bits or raw bytes
- **Endianness**: Little-endian (LSB first)

---

## Byte Map

| Byte | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 | Description |
|------|-------|-------|-------|-------|-------|-------|-------|-------|-------------|
| **0** | Reserved | Reserved | Reserved | Reserved | i2c_write_status | i2c_read_status | i2c_master_slave_status | is_spa_configured | Configuration & I2C Status |
| **1** | i2c_failed_counter [7:0] |||||| (LSB) | I2C Failed Counter (Low Byte) |
| **2** | i2c_failed_counter [15:8] |||||| (MSB) | I2C Failed Counter (High Byte) |
| **3** | wifi_reconn_counter [7:0] |||||| (LSB) | WiFi Reconnection Counter (Low Byte) |
| **4** | wifi_reconn_counter [15:8] |||||| (MSB) | WiFi Reconnection Counter (High Byte) |
| **5** | mqtt_reconn_counter [7:0] |||||| (LSB) | MQTT Reconnection Counter (Low Byte) |
| **6** | mqtt_reconn_counter [15:8] |||||| (MSB) | MQTT Reconnection Counter (High Byte) |
| **7** | light2_set_cmd_status | light1_set_cmd_status | pump6_set_cmd_status | pump5_set_cmd_status | pump4_set_cmd_status | pump3_set_cmd_status | pump2_set_cmd_status | pump1_set_cmd_status | Pump & Light SET Command Status |
| **8** | resetWifiSta_set_cmd_status | ecoCycle_set_cmd_status | filterCycle_set_cmd_status | setTemp_set_cmd_status | time_set_cmd_status | tempScale_set_cmd_status | waterfall_set_cmd_status | blower_set_cmd_status | Configuration SET Command Status |
| **9** | Reserved | Reserved | Reserved | clockMode_set_cmd_status | tempRange_set_cmd_status | heatMode_set_cmd_status | keypadState_set_cmd_status | errorCodeURL_set_cmd_status | Advanced SET Command Status |
| **10** | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved |
| **11** | setupInfo_get_cmd_status | ecoCycle_get_cmd_status | filterCycle_get_cmd_status | currentTemp_get_cmd_status | setTemp_get_cmd_status | time_get_cmd_status | tempScale_get_cmd_status | deviceStatus_get_cmd_status | Device GET Command Status |
| **12** | Reserved | Reserved | Reserved | Reserved | tempRange_get_cmd_status | heatMode_get_cmd_status | keypadState_get_cmd_status | fwVersion_get_cmd_status | System GET Command Status |
| **13** | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved |
| **14** | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved |
| **15** | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | ota_cmd_status | OTA Command Status |
| **16** | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved | Reserved |

---

## Detailed Field Descriptions

### Byte 0: Configuration & I2C Status

| Bit | Field Name | Type | Description |
|-----|------------|------|-------------|
|  0  | is_spa_configured | bool | Indicates if spa is configured |
|  1  | i2c_master_slave_status | enum | I2C role: 0=Slave, 1=Master |
|  2  | i2c_read_status | bool | Last I2C read operation status |
|  3  | i2c_write_status | bool | Last I2C write operation status |
| 4-7 | reserved1 | - | Reserved for future use |

### Bytes 1-2: I2C Failed Counter

| Field | Range | Type | Description |
|-------|-------|------|-------------|
| i2c_failed_counter | 0-65535 | uint16_t | Number of I2C communication failures (auto-wraps at 65535) |

### Bytes 3-4: WiFi Reconnection Counter

| Field | Range | Type | Description |
|-------|-------|------|-------------|
| wifi_reconn_counter | 0-65535 | uint16_t | Number of WiFi reconnection attempts (auto-wraps at 65535) |

### Bytes 5-6: MQTT Reconnection Counter

| Field | Range | Type | Description |
|-------|-------|------|-------------|
| mqtt_reconn_counter | 0-65535 | uint16_t | Number of MQTT reconnection attempts (auto-wraps at 65535) |

### Byte 7: Pump & Light SET Command Status

| Bit | Field Name | Description |
|-----|------------|-------------|
|  0  | pump1_set_cmd_status | Pump 1 SET command execution status |
|  1  | pump2_set_cmd_status | Pump 2 SET command execution status |
|  2  | pump3_set_cmd_status | Pump 3 SET command execution status |
|  3  | pump4_set_cmd_status | Pump 4 SET command execution status |
|  4  | pump5_set_cmd_status | Pump 5 SET command execution status |
|  5  | pump6_set_cmd_status | Pump 6 SET command execution status |
|  6  | light1_set_cmd_status | Light 1 SET command execution status |
|  7  | light2_set_cmd_status | Light 2 SET command execution status |

### Byte 8: Configuration SET Command Status

| Bit | Field Name | Description |
|-----|------------|-------------|
|  0  | blower_set_cmd_status | Blower SET command execution status |
|  1  | waterfall_set_cmd_status | Waterfall SET command execution status |
|  2  | tempScale_set_cmd_status | Temperature scale SET command status |
|  3  | time_set_cmd_status | Time SET command execution status |
|  4  | setTemp_set_cmd_status | Set temperature command execution status |
|  5  | filterCycle_set_cmd_status | Filter cycle SET command execution status |
|  6  | ecoCycle_set_cmd_status | Eco cycle SET command execution status |
|  7  | resetWifiSta_set_cmd_status | WiFi station reset command execution status |

### Byte 9: Advanced SET Command Status

| Bit | Field Name | Description |
|-----|------------|-------------|
|  0  | errorCodeURL_set_cmd_status | Error code URL SET command execution status |
|  1  | keypadState_set_cmd_status | Keypad state SET command execution status |
|  2  | heatMode_set_cmd_status | Heat mode SET command execution status |
|  3  | tempRange_set_cmd_status | Temperature range SET command execution status |
|  4  | clockMode_set_cmd_status | Clock mode SET command execution status |
| 5-7 | reserved2 | Reserved for future use |

### Byte 10: Reserved

All bits reserved for future use.

### Byte 11: Device GET Command Status

| Bit | Field Name | Description |
|-----|------------|-------------|
|  0  | deviceStatus_get_cmd_status | Device status GET command execution status |
|  1  | tempScale_get_cmd_status | Temperature scale GET command execution status |
|  2  | time_get_cmd_status | Time GET command execution status |
|  3  | setTemp_get_cmd_status | Set temperature GET command execution status |
|  4  | currentTemp_get_cmd_status | Current temperature GET command execution status |
|  5  | filterCycle_get_cmd_status | Filter cycle GET command execution status |
|  6  | ecoCycle_get_cmd_status | Eco cycle GET command execution status |
|  7  | setupInfo_get_cmd_status | Setup info GET command execution status |

### Byte 12: System GET Command Status

| Bit | Field Name | Description |
|-----|------------|-------------|
|  0  | fwVersion_get_cmd_status | Firmware version GET command execution status |
|  1  | keypadState_get_cmd_status | Keypad state GET command execution status |
|  2  | heatMode_get_cmd_status | Heat mode GET command execution status |
|  3  | tempRange_get_cmd_status | Temperature range GET command execution status |
| 4-7 | reserved4 | Reserved for future use |

### Bytes 13-14: Reserved

All bits reserved for future use.

### Byte 15: OTA Command Status

| Bit | Field Name | Description |
|-----|------------|-------------|
|  0  | ota_cmd_status | OTA update command execution status |
| 1-7 | reserved5 | Reserved for future use |

### Byte 16: Reserved

All bits reserved for future use.

---

## API Functions

### Control Functions

| Function | Description |
|----------|-------------|
| `wireless_debugger_init()` | Initialize debugger (turns OFF by default) |
| `wireless_debugger_on()` | Enable wireless debugging |
| `wireless_debugger_off()` | Disable wireless debugging |
| `is_wireless_debugger_on()` | Check if debugger is enabled |

### Counter Management

| Function | Description |
|----------|-------------|
| `set_wireless_debugger_i2c_failed_counter_up()` | Increment I2C failure counter (wraps at 65535) |
| `set_wireless_debugger_wifi_reconn_counter_up()` | Increment WiFi reconnection counter (wraps at 65535) |
| `set_wireless_debugger_mqtt_reconn_counter_up()` | Increment MQTT reconnection counter (wraps at 65535) |
| `set_wireless_debugger_i2c_failed_counter_reset()` | Reset I2C failure counter to 0 |
| `set_wireless_debugger_wifi_reconn_counter_reset()` | Reset WiFi reconnection counter to 0 |
| `set_wireless_debugger_mqtt_reconn_counter_reset()` | Reset MQTT reconnection counter to 0 |

### Getter Functions

All status bits have corresponding getter functions following the pattern:
- `get_wireless_debugger_<field_name>()` - Returns bool or uint8_t/uint16_t

### Setter Functions

All writable bits have corresponding setter functions following the pattern:
- `set_wireless_debugger_<field_name>(bool status_)` - For boolean fields
- `set_wireless_debugger_i2c_master_slave_status(I2C_ROLE_e role_)` - For I2C role

---

## Known Issues

### Implementation Bugs

The following bugs exist in the current implementation (`wireless_debugger.cpp`):

1. **get_wireless_debugger_pump6_cmd_status()** - Returns `pump1_set_cmd_status` instead of `pump6_set_cmd_status`
2. **get_wireless_debugger_light2_cmd_status()** - Returns `light1_set_cmd_status` instead of `light2_set_cmd_status`

These should be corrected to return the proper bit fields.

---

## Memory Layout

```
Offset  0: [xxxx|i2c_wr|i2c_rd|i2c_m/s|is_spa_cfg]
Offset  1: [i2c_failed_counter LOW BYTE]
Offset  2: [i2c_failed_counter HIGH BYTE]
Offset  3: [wifi_reconn_counter LOW BYTE]
Offset  4: [wifi_reconn_counter HIGH BYTE]
Offset  5: [mqtt_reconn_counter LOW BYTE]
Offset  6: [mqtt_reconn_counter HIGH BYTE]
Offset  7: [lt2|lt1|p6|p5|p4|p3|p2|p1]
Offset  8: [wfRst|eco|flt|sT|tm|tSc|wtf|blw]
Offset  9: [xxx|clk|tR|hM|kp|err]
Offset 10: [reserved]
Offset 11: [setup|eco|flt|cT|sT|tm|tSc|dev]
Offset 12: [xxxx|tR|hM|kp|fw]
Offset 13: [reserved]
Offset 14: [reserved]
Offset 15: [xxxxxxx|ota]
Offset 16: [reserved]

```