#ifndef SPACONTROL_H
#define SPACONTROL_H

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include "../../src/main.h"
#include <balboa.h>

typedef struct{
  int filter1StartHour;
  int filter1StartMinute;
  int filter1DurationHour;
  int filter1DurationMinute;
  int filter2StartHour;
  int filter2StartMinute;
  int filter2DurationHour;
  int filter2DurationMinute;
}filterCycleData_t;


typedef struct {
  bool is_mqtt_topic_postfix_present;
  char mqtt_topic_postfix[128];
} mqtt_params_t;

typedef struct 
{
  float setTemp;
  float measuredTemp;
}spaTempData;

typedef struct {
  bool  is_jet1_present;
  bool  jet1;

  bool  is_jet2_present;
  bool  jet2;

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
} spaControlParams_t;

typedef struct {
  bool deviceStatus;
  
  bool bootupPacket;
  bool currentTemp;
  bool setTemp;
  bool heatMode;
  bool tempRange;

  bool device_info;
  int device_id;
  int user_id;

  bool filterCycle;
  bool filter1;
  bool filter2;
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

#endif