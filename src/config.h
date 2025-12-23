// Update as needed and copy to config.h

#define FW_REV_STR  "2025-12-23-v.1.27"

#define WIFI_SSID           "Spapilot Reset Hotspot"
#define WIFI_PASSWORD       "Zavepwr21#&"

#define WIFI_AP_SSID        "Spapilot (Balboa)"
// #define WIFI_AP_SSID        "Spapilot Setup WiFi"
#define WIFI_AP_PASSWORD    "" //"Zavepwr21#&"

#define GMT_OFFSET -14400
#define DAYLIGHT_OFFSET 0

#define AUTO_TX true

// Used by LOCAL_CLIENT - rs485 connection
#ifdef ARDUINO_ESP32S3_DEV
#define TX485_Rx 18          // esp32s3
#else
#define TX485_Rx 16             // esp32
#endif
#define TX485_Tx 17

/* MQTT creds */
#define USE_MQTTS
#ifdef USE_MQTTS
#define xUSE_COREFRAGMENT_MQTT_CREDS
#endif