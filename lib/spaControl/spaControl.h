#ifndef SPACONTROL_H
#define SPACONTROL_H

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include "../../src/main.h"
#include <balboa.h>
// #include <Preferences.h>

// extern Preferences cliUrl;

extern String clientUrl;

// extern uint8_t hour;
// extern uint8_t minute;
// extern uint8_t cleanupCycleTime;
extern char j;

typedef struct {
  bool is_mqtt_topic_postfix_present;
  char mqtt_topic_postfix[128];

  bool is_parse_mqtt_msg_present;
  bool parse_mqtt_msg;
  char mqtt_msg[256];
  int mqtt_msg_len;
} mqtt_params_t;


typedef struct {
  bool  is_jet1_present;
  bool  jet1;

  bool  is_jet2_present;
  bool  jet2;

  bool  is_jet3_present;
  bool  jet3;

  bool  is_jet4_present;
  bool  jet4;

  bool  is_blower1_present;
  bool  blower1;
  
  bool  is_light1_present;
  bool  light1;

  bool  is_reset_wifi_sta_present;
  bool  reset_wifi_sta;

  bool is_set_temp_present;
  bool setTempCommand;

  bool is_temp_range_high_present;
  bool setTempRangeHigh;
  bool is_temp_range_low_present;
  bool setTempRangeLow;

  bool is_ready_mode_present;
  bool setModeReady;
  bool is_resting_mode_present;
  bool setModeRest;

  bool is_filterCycle_present;
  bool filterCycle;

  bool  is_hold_present;
  bool  hold;

  bool  is_time_present;
  bool  time;

  bool is_clockMode_present;

  bool is_tempScale_present;
  bool tempScale;
  
  bool is_m8_present;

  bool is_cleanupCycle_present;
} spaControlParams_t;

typedef struct {
  bool deviceStatus;
  
  bool ota_stat;
  bool bootupPacket;
  bool currentTemp;
  bool setTemp;
  bool heatMode;
  bool tempRange;
  bool heatingState;
  bool device_info;
  int device_id;
  int user_id;
  bool setupInfo;
  bool filterCycle;
  bool filter1;
  bool filter2;
  bool fwVersion;
  bool hold;
  bool tempScale;
} spaControlStatus_t;

typedef struct {
  bool  is_url_present;
  char  url[512];
} otaParams_t;



void startSpaCmdSendTimer();
void stopSpaCmdSendTimer();

void set_mqtt_params(mqtt_params_t _mqtt_params);
spaControlParams_t get_spaControlParams(void);
void set_spaControlParams(spaControlParams_t _spaControlParams);
spaControlStatus_t get_spaControlStatus(void);
void set_spaControlStatus(spaControlStatus_t _spaControlStatus);

void spaControl_action(void);
void spaControl_mqtt_action(void);
bool spaControl_parse_action_command(char *json_str, spaControlParams_t *spaControlParams, spaControlStatus_t *spaControlStatus, otaParams_t *otaParams);
void spaControl_create_deviceStatus(SpaStatusData _SpaStatusData, char *json_str);
void spaControl_create_currentTemp(SpaStatusData _SpaStatusData, char *json_str);
void spaControl_create_setTemp(SpaStatusData _SpaStatusData, char *json_str);
void spaControl_create_heatMode(SpaStatusData _SpaStatusData, char *json_str);
void spaControl_create_tempRange(SpaStatusData _SpaStatusData, char *json_str);
void spaControl_create_bootupPacket(char *json_str);
void spaControl_create_setupInfo(SpaInformationData spa_information_data, char *json_str);
void spaControl_create_errorCode_message(char *json_str, uint8_t initMode, uint8_t reminderType);
void spaControl_create_fwVersion(char *json_str);


// extern mqtt_params_t mqtt_params;
void sendOTASuccess(void);
void sendOTAFail(void);
void sendOTAStarted(void);
void toggleHoldState(void);
void configRequest(void);
void setTime(int hour, int minute);
void setCleanupCycle(void);
void setM8_off(void);
void setM8_off_byOTA(void);
#endif