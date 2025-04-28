#ifndef SPACONTROL_H
#define SPACONTROL_H

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include "../../src/main.h"

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

  bool setTempCommand;

  bool setTempRangeHigh;
  bool setTempRangeLow;

  bool setWorkingModeRest;
  bool setWorkingModeReady;
  
} spaControlParams_t;

typedef struct {
  bool deviceStatus;
  bool bootupPacket;
  bool currentTemp;
  bool setTemp;
} spaControlStatus_t;

typedef struct {
  bool  is_url_present;
  char  url[512];
} otaParams_t;

void startSpaCmdSendTimer();
void stopSpaCmdSendTimer();
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
void spaControl_create_bootupPacket(char *json_str);

#endif