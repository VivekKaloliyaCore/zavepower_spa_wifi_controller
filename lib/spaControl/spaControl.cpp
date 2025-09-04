#include <Arduino.h>
#include <CircularBuffer.hpp>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <WiFi.h>

#include <spaUtilities.h>
#include <spaMessage.h>

#include "spaControl.h"
#include "rs485.h"
#include "spaMqttMessage.h"
#include "PubSubClient.h"
#include "mqttModule.h"
#include <wifiModule.h>
#include "../../src/config.h"
#include "../../src/main.h"
#include <balboa.h>
#include <Preferences.h>

StaticJsonDocument<512> globalDeviceInfo; // Adjust size as needed

mqtt_params_t mqtt_params = {0};

// Global variable to send temp to set.
float sendSetTemp = 0;

uint8_t hour = 0;
uint8_t minute = 0;
uint8_t cleanupCycleTime = 0;
uint8_t m8 = 0;

Ticker spaCmdSendTimer;
bool spaCmdSendTimerRunning = false;
bool sendSpaCmdSend = false;

static spaControlParams_t spaControlParams = {0};
static spaControlStatus_t spaControlStatus = {0};

// SpaFilterSettingsData spaFilterSettingsData = {0};

String clientUrl;
Preferences api;

char k = 0;
char j = 0;

// Local Function
void switchTempRange(void);
void switchHeatMode(void);
void toggleJet1(void);
void toggleJet2(void);
void toggleJet3(void);
void toggleJet4(void);
void toggleLight1(void);
void toggleBlower1(void);
void setTemp(float temp);
void informationRequest(void);
void spaControl_appand_device_info(DynamicJsonDocument* doc);
void spaControl_create_filter_cycle(char *json_str);
void spaControl_create_hold_status(char *json_str);
void spaControl_create_tempScale_status(char *json_str);
void spaControl_create_fwVersion(char *json_str);
void setCleanupCycle(void);
void setClockMode(void);
void setTempScale(void);
void setM8(void);
void configRequest(void);

void myFunction()
{
  Log.notice("startSpaCmdSendTimer Triggered!\n");
  sendSpaCmdSend = true;
}

void startSpaCmdSendTimer()
{
  if (!spaCmdSendTimerRunning)
  {
    sendSpaCmdSend = false;
    spaCmdSendTimer.attach(3, myFunction);  // Call every 1 second
    spaCmdSendTimerRunning = true;
    Log.notice("startSpaCmdSendTimer Started!\n");
  }
}

void stopSpaCmdSendTimer()
{
  if (spaCmdSendTimerRunning)
  {
    sendSpaCmdSend = false;
    spaCmdSendTimer.detach();  // Stop the timer
    spaCmdSendTimerRunning = false;
    Log.notice("startSpaCmdSendTimer Stopped!\n");
  }
}

void set_mqtt_params(mqtt_params_t _mqtt_params)
{
  mqtt_params.is_mqtt_topic_postfix_present = _mqtt_params.is_mqtt_topic_postfix_present;
  if(mqtt_params.is_mqtt_topic_postfix_present)
  {
    memset(&mqtt_params.mqtt_topic_postfix[0] ,0, sizeof(mqtt_params.mqtt_topic_postfix));
    memcpy(&mqtt_params.mqtt_topic_postfix[0] ,&_mqtt_params.mqtt_topic_postfix[0], strlen(_mqtt_params.mqtt_topic_postfix));
  }

  mqtt_params.is_parse_mqtt_msg_present = _mqtt_params.is_parse_mqtt_msg_present;
  if(mqtt_params.is_parse_mqtt_msg_present)
  {
    mqtt_params.parse_mqtt_msg = _mqtt_params.parse_mqtt_msg;
    mqtt_params.mqtt_msg_len = _mqtt_params.mqtt_msg_len;
    memset(&mqtt_params.mqtt_msg[0] ,0, sizeof(mqtt_params.mqtt_msg));
    memcpy(&mqtt_params.mqtt_msg[0] ,&_mqtt_params.mqtt_msg[0], strlen(_mqtt_params.mqtt_msg));
  }
}

spaControlParams_t get_spaControlParams(void)
{
  return spaControlParams;
}

void set_spaControlParams(spaControlParams_t _spaControlParams)
{
  stopSpaCmdSendTimer();
  spaControlParams.is_jet1_present = _spaControlParams.is_jet1_present;
  spaControlParams.jet1 = _spaControlParams.jet1;

  spaControlParams.is_jet2_present = _spaControlParams.is_jet2_present;
  spaControlParams.jet2 = _spaControlParams.jet2;

  spaControlParams.is_jet3_present = _spaControlParams.is_jet3_present;
  spaControlParams.jet3 = _spaControlParams.jet3;

  spaControlParams.is_jet4_present = _spaControlParams.is_jet4_present;
  spaControlParams.jet4 = _spaControlParams.jet4;

  spaControlParams.is_blower1_present = _spaControlParams.is_blower1_present;
  spaControlParams.blower1 = _spaControlParams.blower1;

  spaControlParams.is_light1_present = _spaControlParams.is_light1_present;
  spaControlParams.light1 = _spaControlParams.light1;

  spaControlParams.is_temp_range_high_present = _spaControlParams.is_temp_range_high_present;
  spaControlParams.setTempRangeHigh = _spaControlParams.setTempRangeHigh;

  spaControlParams.is_temp_range_low_present = _spaControlParams.is_temp_range_low_present;
  spaControlParams.setTempRangeLow = _spaControlParams.setTempRangeLow;

  spaControlParams.is_ready_mode_present = _spaControlParams.is_ready_mode_present;
  spaControlParams.setModeReady = _spaControlParams.setModeReady;

  spaControlParams.is_resting_mode_present = _spaControlParams.is_resting_mode_present;
  spaControlParams.setModeRest = _spaControlParams.setModeRest;

  spaControlParams.is_set_temp_present = _spaControlParams.is_set_temp_present;
  spaControlParams.setTempCommand = _spaControlParams.setTempCommand;

  spaControlParams.is_filterCycle_present = _spaControlParams.is_filterCycle_present;
  spaControlParams.filterCycle = _spaControlParams.filterCycle;

  spaControlParams.is_hold_present = _spaControlParams.is_hold_present;
  spaControlParams.hold = _spaControlParams.hold;

  spaControlParams.is_time_present = _spaControlParams.is_time_present;

  spaControlParams.is_clockMode_present = _spaControlParams.is_clockMode_present;

  spaControlParams.is_tempScale_present = _spaControlParams.is_tempScale_present;
  spaControlParams.tempScale = _spaControlParams.tempScale;

  spaControlParams.is_m8_present = _spaControlParams.is_m8_present;

  spaControlParams.is_cleanupCycle_present = _spaControlParams.is_cleanupCycle_present;
  // spaControlParams.is_reset_wifi_sta_present = _spaControlParams.is_reset_wifi_sta_present;
  // spaControlParams.reset_wifi_sta = _spaControlParams.reset_wifi_sta;
}

spaControlStatus_t get_spaControlStatus(void)
{
  return spaControlStatus;
}

void set_spaControlStatus(spaControlStatus_t _spaControlStatus)
{
  spaControlStatus.ota_stat = _spaControlStatus.ota_stat;
  spaControlStatus.deviceStatus = _spaControlStatus.deviceStatus;
  spaControlStatus.bootupPacket = _spaControlStatus.bootupPacket;
  spaControlStatus.currentTemp = _spaControlStatus.currentTemp;
  spaControlStatus.setTemp = _spaControlStatus.setTemp;
  spaControlStatus.tempRange = _spaControlStatus.tempRange;
  spaControlStatus.heatMode = _spaControlStatus.heatMode;
  spaControlStatus.device_info = _spaControlStatus.device_info;
  spaControlStatus.device_id = _spaControlStatus.device_id;
  spaControlStatus.user_id = _spaControlStatus.user_id;
  spaControlStatus.heatingState = _spaControlStatus.heatingState;
  spaControlStatus.filterCycle = _spaControlStatus.filterCycle;
  spaControlStatus.setupInfo = _spaControlStatus.setupInfo;
  spaControlStatus.filter1 = _spaControlStatus.filter1;
  spaControlStatus.filter2 = _spaControlStatus.filter2;
  spaControlStatus.fwVersion = _spaControlStatus.fwVersion;
  spaControlStatus.hold = _spaControlStatus.hold;
  spaControlStatus.tempScale = _spaControlStatus.tempScale;
}


void spaControl_action(void)
{
  // if(spaControlParams.setTempCommand)
  // {
  //   spaControlParams.setTempCommand = false;
  //   setTemp(0x1E);
  // }

  if(j == 1)
  {
    spaControlStatus.setupInfo = true;
    j++;
  }
  if(mqtt_params.parse_mqtt_msg)
  {
    mqtt_params.is_parse_mqtt_msg_present = false;
    mqtt_params.parse_mqtt_msg = false;

    spaControlParams_t spaControlParams = {0};
    memset(&spaControlParams, 0, sizeof(spaControlParams_t));

    spaControlStatus_t spaControlStatus = {0};
    memset(&spaControlStatus, 0, sizeof(spaControlStatus_t));

    otaParams_t otaParams = {0};
    memset(&otaParams, 0, sizeof(otaParams_t));

    if(spaControl_parse_action_command((char *)mqtt_params.mqtt_msg, &spaControlParams, &spaControlStatus, &otaParams))
    {
      Log.notice("parse_action_command\n");
      if(spaControlStatus.deviceStatus)
      {
        Log.notice("Sending deviceStatus...\n");
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.currentTemp)
      {
        Log.notice("Sending currentTemp...\n");
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.setTemp)
      {
        Log.notice("Sending setTemp...\n");
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.heatMode)
      {
        Log.notice("Sending heatMode...\n");
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.tempRange)
      {
        Log.notice("Sending tempRange...\n");
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.device_info)
      {
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.setupInfo)
      {
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.filterCycle)
      {
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.hold)
      {
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.tempScale)
      {
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.fwVersion)
      {
        set_spaControlStatus(spaControlStatus);
      }
      else if(spaControlStatus.ota_stat)
      {
        set_spaControlStatus(spaControlStatus);
      }

      if(spaControlParams.is_jet1_present)
      {
        Log.notice("Sending jet 1 command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_jet2_present)
      {
        Log.notice("Sending jet 2 command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_jet3_present)
      {
        Log.notice("Sending jet 3 command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_jet4_present)
      {
        Log.notice("Sending jet 4 command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_blower1_present)
      {
        Log.notice("Sending Blower 1 command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_light1_present)
      {
        Log.notice("Sending Light 1 command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_reset_wifi_sta_present)
      {
        Log.notice("Reset WiFi STA...\n");

        if(spaControlParams.reset_wifi_sta)
        {
          wifiModuleEraseStaConfig();

          Log.notice("WiFi STA config reset. Rebooting...\n");
          delay(2000);
          ESP.restart();
        }

        // set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_temp_range_high_present)
      {
        // Log.notice("Sending currentTemp...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_temp_range_low_present)
      {
        // Log.notice("Sending currentTemp...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_ready_mode_present)
      {
        // Log.notice("Sending currentTemp...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_resting_mode_present)
      {
        // Log.notice("Sending currentTemp...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_set_temp_present)
      {
        // Log.notice("Sending currentTemp...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_filterCycle_present)
      {
        // Log.notice("Sending currentTemp...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_hold_present)
      {
        Log.notice("Sending Hold command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_time_present)
      {
        Log.notice("Sending time command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_clockMode_present)
      {
        Log.notice("Sending time command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_tempScale_present)
      {
        Log.notice("Sending time command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_m8_present)
      {
        Log.notice("Sending time command...\n");
        set_spaControlParams(spaControlParams);
      }
      else if(spaControlParams.is_cleanupCycle_present)
      {
        Log.notice("Sending time command...\n");
        set_spaControlParams(spaControlParams);
      }

      if(otaParams.is_url_present)
      {
        String message;
        for (unsigned int i = 0; i < strlen(otaParams.url); i++) {
            message += (char)otaParams.url[i];
        }
    
        Log.notice("Firmware URL: %s\n", message.c_str());
        
        if(strstr(message.c_str(), "https"))
        {
          performOTA(message);
        }
        else
        {
          performOTA_unsecured(message);
        }

      }
    }
  }


  if(spaControlParams.is_jet1_present)
  {
    if(spaControlParams.jet1)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        toggleJet1();

        sendSpaCmdSend = false;
        startSpaCmdSendTimer();
      }
    }
    else
    {
      sendSpaCmdSend = false;
      stopSpaCmdSendTimer();
    }
  }
  else if(spaControlParams.is_jet2_present)
  {
    if(spaControlParams.jet2)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        toggleJet2();

        sendSpaCmdSend = false;
        startSpaCmdSendTimer();
      }
    }
    else
    {
      sendSpaCmdSend = false;
      stopSpaCmdSendTimer();
    }
  }
  else if(spaControlParams.is_jet3_present)
  {
    if(spaControlParams.jet3)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        toggleJet3();

        sendSpaCmdSend = false;
        startSpaCmdSendTimer();
      }
    }
    else
    {
      sendSpaCmdSend = false;
      stopSpaCmdSendTimer();
    }
  }
  else if(spaControlParams.is_jet4_present)
  {
    if(spaControlParams.jet4)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        toggleJet4();

        sendSpaCmdSend = false;
        startSpaCmdSendTimer();
      }
    }
    else
    {
      sendSpaCmdSend = false;
      stopSpaCmdSendTimer();
    }
  }
  else if(spaControlParams.is_blower1_present)
  {
    if(spaControlParams.blower1)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        toggleBlower1();

        sendSpaCmdSend = false;
        startSpaCmdSendTimer();
      }
    }
    else
    {
      sendSpaCmdSend = false;
      stopSpaCmdSendTimer();
    }
  }
  else if(spaControlParams.is_light1_present)
  {
    if(spaControlParams.light1)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        toggleLight1();
        sendSpaCmdSend = false;
        startSpaCmdSendTimer();
      }
    }
    else
    {
      sendSpaCmdSend = false;
      stopSpaCmdSendTimer();
    }
  }
  else if(spaControlParams.is_temp_range_high_present)
  {
    if(spaControlParams.setTempRangeHigh)
    {
      // Log.notice("ABAB::::::::::\n");
      if(spaStatusData.tempRange == 0) // If Temp range is already Low
      {
        // Log.notice("XYZ::::::::::\n");
        spaControlParams.setTempRangeHigh = false;
        switchTempRange();
      }
    }
  }
  else if(spaControlParams.is_temp_range_low_present)
  {
    // Log.notice("HEHE::::::::::\n");
    if(spaControlParams.setTempRangeLow)
    {
      if(spaStatusData.tempRange == 1) // If Temp range is already High
      {
        spaControlParams.setTempRangeLow = false;
        switchTempRange();
      }
    }
  }
  else if(spaControlParams.is_resting_mode_present)
  {
    if(spaControlParams.setModeRest)
    {
      if(spaStatusData.heatingMode != 0x01)
      {
        spaControlParams.setModeRest = false;
        switchHeatMode();
      }
    }
  }
  else if(spaControlParams.is_ready_mode_present)
  {
    if(spaControlParams.setModeReady)
    {
      if(spaStatusData.heatingMode != 0x00)
      {
        spaControlParams.setModeReady = false;
        switchHeatMode();
      }
    }
  }
  else if(spaControlParams.is_set_temp_present)
  {
    if(spaControlParams.setTempCommand)
    {
      spaControlParams.setTempCommand = false;
      setTemp(sendSetTemp);
    }
  }
  else if(spaControlParams.is_filterCycle_present)
  {
    if(spaControlParams.filterCycle)
    {
      spaControlParams.filterCycle = false;
      filterCycleTrial();
    }

  }
  else if(spaControlParams.is_hold_present)
  {
    if(spaControlParams.hold)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        toggleHoldState();

        sendSpaCmdSend = false;
        startSpaCmdSendTimer();
      }
    }
    else
    {
      sendSpaCmdSend = false;
      stopSpaCmdSendTimer();
    }
  }
  else if(spaControlParams.is_time_present)
  {
    setTime(hour, minute);
    spaControlParams.is_time_present = false;
  }
  else if(spaControlParams.is_clockMode_present)
  {
    setClockMode();
    spaControlParams.is_clockMode_present = false;
  }
  else if(spaControlParams.is_tempScale_present)
  {
    setTempScale();
    // spaControlParams.is_tempScale_present = false;
  }
  else if(spaControlParams.is_m8_present)
  {
    setM8();
    spaControlParams.is_m8_present = false;
  }
  else if(spaControlParams.is_cleanupCycle_present)
  {
    setCleanupCycle();
    spaControlParams.is_cleanupCycle_present = false;
  }
  // else if(spaControlParams.is_reset_wifi_sta_present)
  // {
  //   if(spaControlParams.reset_wifi_sta)
  //   {
  //     wifiModuleEraseStaConfig();

  //     Log.notice("WiFi STA config reset. Rebooting...\n");
  //     delay(2000);
  //     ESP.restart();
  //   }
  // }
}

void sendOTAStarted(void)
{
  char json_str[512];
  memset(&json_str[0], 0, sizeof(json_str));
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "ota";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  payload["msg"] = "device upgrading...";

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);
  memcpy(json_str, output.c_str(), strlen(output.c_str()));
  spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
}

void sendOTASuccess(void)
{
  char json_str[512];
  memset(&json_str[0], 0, sizeof(json_str));
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "msg";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  payload["ota"] = "Device upgraded successfully.";

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);
  memcpy(json_str, output.c_str(), strlen(output.c_str()));
  spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
}

void sendOTAFail(void)
{
  char json_str[512];
  memset(&json_str[0], 0, sizeof(json_str));
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "msg";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  payload["ota"] = "device upgrade failed.";

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);
  memcpy(json_str, output.c_str(), strlen(output.c_str()));
  spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
}

void spaControl_mqtt_action(void)
{
  if(spaControlStatus.deviceStatus)
  {
    spaControlStatus.deviceStatus = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_deviceStatus(spa_status_data, json_str);

    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.currentTemp)
  {
    spaControlStatus.currentTemp = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_currentTemp(spa_status_data, json_str);

    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.setTemp)
  {
    spaControlStatus.setTemp = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_setTemp(spa_status_data, json_str);
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.heatMode)
  {
    spaControlStatus.heatMode = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_heatMode(spa_status_data, json_str);
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.setupInfo)
  {
    delay(500);
    spaControlStatus.setupInfo = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaInformationData spa_information_data = spaMessage_get_spaInformationData();
    k = 0;
    spaControl_create_setupInfo(spa_information_data, json_str);
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
    Log.notice(">>>>Setup Info Published\n");
  }

  if(spaControlStatus.tempRange)
  {
    spaControlStatus.tempRange = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_tempRange(spa_status_data, json_str);
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.filterCycle)
  {
    spaControlStatus.filterCycle = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    // SpaFilterSettingsData spaFilterSettingsData = spaMessage_get_spaFilterData();
    // spaControl_create_filter_cycle(json_str);
    // spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.hold)
  {
    spaControlStatus.hold = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    // SpaFilterSettingsData spaFilterSettingsData = spaMessage_get_spaFilterData();
    spaControl_create_hold_status(json_str);
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.tempScale)
  {
    Log.notice("spaControlStatus.tempScale = trueeee\n");
    spaControlStatus.tempScale = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    // SpaFilterSettingsData spaFilterSettingsData = spaMessage_get_spaFilterData();
    spaControl_create_tempScale_status(json_str);
    Log.notice("Created temp scale\n");
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
    Log.notice("Sent temp scale\n");
  }

  if(spaControlStatus.fwVersion)
  {
    spaControlStatus.fwVersion = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    // SpaFilterSettingsData spaFilterSettingsData = spaMessage_get_spaFilterData();
    spaControl_create_fwVersion(json_str);
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.ota_stat)
  {
    spaControlStatus.ota_stat = false;
    sendOTASuccess();
  }

  if(spaControlStatus.bootupPacket)
  {
    spaControlStatus.bootupPacket = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    // SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_bootupPacket(json_str);
    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
    // spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], NULL, 0);
    Log.notice("Boot Up pacage Published\n");
    
    // spaControlStatus.setupInfo = true;// Enable this to send setupinfo at the time of bootup.
  }


}

bool spaControl_parse_action_command(char *json_str, spaControlParams_t *spaControlParams, spaControlStatus_t *spaControlStatus, otaParams_t *otaParams)
{
  // Create a JSON document
  DynamicJsonDocument doc(500);

  // Parse the JSON string
  DeserializationError error = deserializeJson(doc, json_str);
  if (error)
  {
    Log.notice("Deserialization failed: %s\n", error.f_str());
    return false;
  }

  /* Extract "action" */
  const char* action = doc["action"];
  Log.notice("Action: %s\n", action);

  /* action 'set' */
  if(strstr(action, "set"))
  {
    if (doc.containsKey("payload"))
    {
      if(doc["payload"].containsKey("tempRange"))
      {
        String tempRange = doc["payload"]["tempRange"];
        Log.notice("Temperature Range: %s\n", tempRange);

        if(tempRange == "high")
        {
          spaControlParams->is_temp_range_high_present = true;
          spaControlParams->setTempRangeHigh = true;
          spaControlParams->setTempRangeLow = false;
        }
        else if(tempRange == "low")
        {
          spaControlParams->is_temp_range_low_present = true;
          spaControlParams->setTempRangeHigh = false;
          spaControlParams->setTempRangeLow = true;
        }
      }

      else if(doc["payload"].containsKey("heatMode"))
      {
        String heatMode = doc["payload"]["heatMode"];
        Log.notice("Heat Mode: %s\n", heatMode);

        if(heatMode == "rest")
        {
          spaControlParams->is_resting_mode_present = true;
          spaControlParams->setModeRest = true;
          spaControlParams->setModeReady = false;
        }
        else if(heatMode == "ready")
        {
          spaControlParams->is_ready_mode_present = true;
          spaControlParams->setModeRest = false;
          spaControlParams->setModeReady = true;
        }
      }

      else if(doc["payload"].containsKey("setTemp"))
      {
        sendSetTemp = doc["payload"]["setTemp"];
        Log.notice("Temperature Range: %s\n", sendSetTemp);

        if(spaStatusData.tempRange == 1) // Set temp range is HIGH. i.e. : 26.5 degree celcius to 40 degree celcius
        {
          if(spaStatusData.tempScale == 1)
          {
            if(sendSetTemp >= 26.5 && sendSetTemp <= 40)
            {
              spaControlParams->is_set_temp_present = true;
              spaControlParams->setTempCommand = true;
            }
          }
          else if(spaStatusData.tempScale == 0)
          {
            if(sendSetTemp >= 80 && sendSetTemp <= 104)
            {
              spaControlParams->is_set_temp_present = true;
              spaControlParams->setTempCommand = true;
            }
          }
          else
          {
            spaControlParams->is_set_temp_present = false;
            spaControlParams->setTempCommand = false;
          }
        }
        else if(spaStatusData.tempRange == 0) // Set temp range is LOW. i.e. : 10 degree celcius to 37 degree celcius
        {
          if(spaStatusData.tempScale == 1)
          {
            if(sendSetTemp >= 10 && sendSetTemp <= 37)
            {
              spaControlParams->is_set_temp_present = true;
              spaControlParams->setTempCommand = true;
            }
          }
          else if(spaStatusData.tempScale == 0)
          {
            if(sendSetTemp >= 50 && sendSetTemp <= 99)
            {
              spaControlParams->is_set_temp_present = true;
              spaControlParams->setTempCommand = true;
            }
          }
          else
          {
            spaControlParams->is_set_temp_present = false;
            spaControlParams->setTempCommand = false;
          }
        }
        else
        {
          spaControlParams->is_set_temp_present = false;
          spaControlParams->setTempCommand = false;
        }
      }

     else if(doc["payload"].containsKey("jet1"))
      {
        int jet1 = doc["payload"]["jet1"];
        Log.notice("Jet1: %d\n", jet1);

        spaControlParams->is_jet1_present = true;
        spaControlParams->jet1 = jet1;
      }
      else if(doc["payload"].containsKey("jet2"))
      {
        int jet2 = doc["payload"]["jet2"];
        Log.notice("Jet2: %d\n", jet2);

        spaControlParams->is_jet2_present = true;
        spaControlParams->jet2 = jet2;
      }
      else if(doc["payload"].containsKey("jet3"))
      {
        if(spaConfigurationData.pump3)
        {
          int jet3 = doc["payload"]["jet3"];
          Log.notice("Jet3: %d\n", jet3);

          spaControlParams->is_jet3_present = true;
          spaControlParams->jet3 = jet3;
        }
      }
      else if(doc["payload"].containsKey("jet4"))
      {
        if(spaConfigurationData.pump4)
        {
          int jet4 = doc["payload"]["jet4"];
          Log.notice("Jet4: %d\n", jet4);

          spaControlParams->is_jet4_present = true;
          spaControlParams->jet4 = jet4;
        }
      }
      else if(doc["payload"].containsKey("blower1"))
      {
       if(spaConfigurationData.blower)
       {
        int blower1 = doc["payload"]["blower1"];
        Log.notice("Blower1: %d\n", blower1);

        spaControlParams->is_blower1_present = true;
        spaControlParams->blower1 = blower1;
       }
      }
      else if(doc["payload"].containsKey("light1"))
      {
        int light1 = doc["payload"]["light1"];
        Log.notice("Light1: %d\n", light1);

        spaControlParams->is_light1_present = true;
        spaControlParams->light1 = light1;
      }
      else if(doc["payload"].containsKey("reset_wifi_sta"))
      {
        int reset_wifi_sta = doc["payload"]["reset_wifi_sta"];
        Log.notice("reset_wifi_sta: %d\n", reset_wifi_sta);

        spaControlParams->is_reset_wifi_sta_present = true;
        spaControlParams->reset_wifi_sta = reset_wifi_sta;
      }
      // else if(doc["payload"].containsKey("filterCycle"))
      // {
      //   spaControlParams->is_filterCycle_present = true;
      //   spaControlParams->filterCycle = true;
      //   if(doc["payload"]["filterCycle"].containsKey("1"))
      //   {
      //     Log.notice("Filter 1 Received\n");
      //     spaFilterSettingsData.filt1Hour = doc["payload"]["filterCycle"]["1"]["startTimeHr"];
      //     spaFilterSettingsData.filt1Minute = doc["payload"]["filterCycle"]["1"]["startTimeMin"];
      //     spaFilterSettingsData.filt1DurationHour = doc["payload"]["filterCycle"]["1"]["durationHr"];
      //     spaFilterSettingsData.filt1DurationMinute = doc["payload"]["filterCycle"]["1"]["durationMin"];
      //   }
      //   if(doc["payload"]["filterCycle"].containsKey("2"))
      //   {

      //     Log.notice("Filter 2 Received\n");
      //     if(doc["payload"]["filterCycle"]["2"]["enable"] == true)
      //     {
      //       spaFilterSettingsData.filt2Enable = 1;
      //       spaFilterSettingsData.filt2Hour = doc["payload"]["filterCycle"]["2"]["startTimeHr"];
      //       spaFilterSettingsData.filt2Minute = doc["payload"]["filterCycle"]["2"]["startTimeMin"];
      //       spaFilterSettingsData.filt2DurationHour = doc["payload"]["filterCycle"]["2"]["durationHr"];
      //       spaFilterSettingsData.filt2DurationMinute = doc["payload"]["filterCycle"]["2"]["durationMin"];
      //     }
      //     else
      //     { 
      //       spaFilterSettingsData.filt2Enable = 0;
      //       spaFilterSettingsData.filt2Hour = 0;
      //       spaFilterSettingsData.filt2Minute = 0;
      //       spaFilterSettingsData.filt2DurationHour = 0;
      //       spaFilterSettingsData.filt2DurationMinute = 0;
      //     }
      //   }
      //   else
      //   {
      //     spaFilterSettingsData.filt2Enable = 0;
      //     spaFilterSettingsData.filt2Hour = 0;
      //     spaFilterSettingsData.filt2Minute = 0;
      //     spaFilterSettingsData.filt2DurationHour = 0;
      //     spaFilterSettingsData.filt2DurationMinute = 0;
      //   }
      // }
      else if(doc["payload"].containsKey("errorCodeURL"))
      {
        String url = doc["payload"]["errorCodeURL"];

        api.begin("myapi", false);
        api.putString("apiurl", url);
        api.end();
        // Log.notice("Received URL: %s\n", api.getString("apiurl", ""));
        delay(1000);

        // api.begin("myapi", true);
        // clientUrl = api.getString("apiurl", "Default");
        // api.end();


        // clientUrl = url;
        // cliUrl.putString("clientUrl", clientUrl);
        // Log.notice("Received Client URL: %s\n", url.c_str());
      }
      else if(doc["payload"].containsKey("hold"))
      {
        spaControlParams->is_hold_present = true;
        // spaControlStatus->hold = true;
        spaControlParams->hold = true;
      }
      else if(doc["payload"].containsKey("time"))
      {
        uint8_t hr = doc["payload"]["time"]["hour"];
        uint8_t min = doc["payload"]["time"]["minute"];

        if(hr <= 23 && min <= 59)
        {
          hour = hr;
          minute = min;
          spaControlParams->is_time_present = true;
        }
        else
        {
          hour = hour;
          minute = minute;
          spaControlParams->is_time_present = false;
        }
      }
      else if(doc["payload"].containsKey("clockMode"))
      {
        spaStatusData.clockMode = doc["payload"]["clockMode"];
        
        spaControlParams->is_clockMode_present = true;
      }
      else if(doc["payload"].containsKey("tempScale"))
      {
        spaStatusData.tempScale = doc["payload"]["tempScale"];
        
        spaControlParams->is_tempScale_present = true;
        spaControlParams->tempScale = true;

      }
      else if(doc["payload"].containsKey("m8"))
      {
        m8 = doc["payload"]["m8"];
        
        spaControlParams->is_m8_present = true;
      }
      else if(doc["payload"].containsKey("cleanupCycle"))
      {
        float cleanmin = doc["payload"]["cleanupCycle"];

        if(cleanmin == 0)
        {
          cleanupCycleTime = 0;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 0.5)
        {
          cleanupCycleTime = 1;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 1)
        {
          cleanupCycleTime = 2;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 1.5)
        {
          cleanupCycleTime = 3;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 2)
        {
          cleanupCycleTime = 4;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 2.5)
        {
          cleanupCycleTime = 5;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 3)
        {
          cleanupCycleTime = 6;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 3.5)
        {
          cleanupCycleTime = 7;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else if(cleanmin == 4)
        {
          cleanupCycleTime = 8;
          spaControlParams->is_cleanupCycle_present = true;
        }
        else
        {
          cleanupCycleTime = cleanupCycleTime;
          spaControlParams->is_cleanupCycle_present = false;
        }
        
        // if(cleanmin <= 8 )
        // {
        //   cleanupCycleTime = cleanmin;
        //   spaControlParams->is_cleanupCycle_present = true;
        // }
        // else
        // {
        //   cleanupCycleTime = cleanupCycleTime;
        //   spaControlParams->is_cleanupCycle_present = false;
        // }
      }
    }
    else
      return false;
  }
  /* action 'get' */
  else if(strstr(action, "get"))
  {
    Log.notice("inside get request!\n");
    if (doc.containsKey("payload"))
    {
      if(doc["payload"].containsKey("deviceStatus"))
      {
        int deviceStatus = doc["payload"]["deviceStatus"];
        Log.notice("deviceStatus: %d\n", deviceStatus);

        spaControlStatus->deviceStatus = true;
      }
      else if(doc["payload"].containsKey("currentTemp"))
      {
        int currentTemp = doc["payload"]["currentTemp"];
        Log.notice("currentTemp: %d\n", currentTemp);

        spaControlStatus->currentTemp = true;
      }
      else if(doc["payload"].containsKey("setTemp"))
      {
        int setTemp = doc["payload"]["setTemp"];
        Log.notice("setTemp: %d\n", setTemp);

        spaControlStatus->setTemp = true;
      }
      else if(doc["payload"].containsKey("heatMode"))
      {
        int heatMode = doc["payload"]["heatMode"];
        Log.notice("Heat Mode : %d\n", heatMode);

        spaControlStatus->heatMode = true;
      }
      else if(doc["payload"].containsKey("tempRange"))
      {
        int tempRange = doc["payload"]["tempRange"];
        Log.notice("Temp range : %d\n", tempRange);

        spaControlStatus->tempRange = true;
      }
      else if(doc["payload"].containsKey("setupInfo"))
      {
        // informationRequest();
        // configRequest();
        spaControlStatus->setupInfo = true;
        // delay(100);
      }
      // else if(doc["payload"].containsKey("filterCycle"))
      // {
      //   spaControlStatus->filterCycle = true;
      //   if(doc["payload"]["filterCycle"].containsKey("cycle1"))
      //   {
      //     spaControlStatus->filter1 = true;
      //   }
      //   if(doc["payload"]["filterCycle"].containsKey("cycle2"))
      //   {
      //     spaControlStatus->filter2 = true;
      //   }
      // }
      else if(doc["payload"].containsKey("fwVersion"))
      {
          spaControlStatus->fwVersion = true;
      }
      else if(doc["payload"].containsKey("holdStatus"))
      {
        spaControlStatus->hold = true;
      }
      else if(doc["payload"].containsKey("tempScale"))
      {
        spaControlStatus->tempScale = true;
      }
    }

    if(doc.containsKey("device_info"))
    {
      spaControlStatus->device_info = true;

      JsonObject deviceInfo = doc["device_info"];
      globalDeviceInfo.clear();
      globalDeviceInfo.set(deviceInfo);

      // for(JsonPair kv : deviceInfo)
      // {
      //   char *key = (char *)(kv.key().c_str());
      //   Log.notice(">>>> key: %s | %s | %s\n", key, kv.key().c_str(), kv.key());
      // }
    }

    // else
    //   return false; // Commented for testing;
  }
  /* action 'ota' */
  else if(strstr(action, "ota"))
  {
    if (doc.containsKey("payload"))
    {
      if(doc["payload"].containsKey("url"))
      {
        const char* url = doc["payload"]["url"];

        memcpy(&otaParams->url[0], &url[0], strlen(url));
        Log.notice("url: %d\n", &otaParams->url[0]);

        otaParams->is_url_present = true;
      }
    }
    else
      return false;
  }
  else
  {
    return false;
  }

  return true;
}

void spaControl_appand_device_info(DynamicJsonDocument* doc)
{
  spaControlStatus.device_info = false;

  JsonObject deviceInfo = doc->createNestedObject("device_info");
  for (JsonPair kv : globalDeviceInfo.as<JsonObject>())
  {
    // char *key = (char *)(kv.key().c_str());
    // Log.notice("<<<< key: %s | %s | %s\n", key, kv.key().c_str(), kv.key());
    deviceInfo[kv.key()] = kv.value();
  }
}

void spaControl_create_fwVersion(char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "fwVersion";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  payload["fwVersion"] = FW_REV_STR;

  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_hold_status(char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "holdStatus";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  if(spaStatusData.spaState == 0x05)
  {
    payload["holdStatus"] = "On Hold";
    payload["Remaining Minutes"] = spaStatusData.sensorA;
  }
  else
  {
    payload["holdStatus"] = "Not On Hold";
  }

  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_tempScale_status(char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "tempScale";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  Log.notice("creating payload\n");
  if(spaStatusData.tempScale == 0)
  {
    payload["tempScale"] = "Fahrenheit";
    Log.notice("Created Fahrenheit\n");
  }
  else if(spaStatusData.tempScale == 1)
  {
    payload["tempScale"] = "Celsius";
    Log.notice("Created Celsius\n");
  }

  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_filter_cycle(char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(350);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "filterCycle";

  JsonObject payload = doc.createNestedObject("payload");
  if(spaControlStatus.filter1)
  {
    // spaControlStatus.filter1 = false;
    payload["filter1StartHour"] = spaFilterSettingsData.filt1Hour;
    payload["filter1StartMin"] = spaFilterSettingsData.filt1Minute;
    payload["filter1DurationHour"] = spaFilterSettingsData.filt1DurationHour;
    payload["filter1DurationMin"] = spaFilterSettingsData.filt1DurationMinute;
  }
  if(spaControlStatus.filter2)
  {
    if(spaFilterSettingsData.filt2Enable)
    {
      // spaControlStatus.filter2 = false;
      // spaFilterSettingsData.filt2Hour = spaFilterSettingsData.filt2Hour & ~(1 << 7);
      payload["filter2StartHour"] = spaFilterSettingsData.filt2Hour;
      payload["filter2StartMin"] = spaFilterSettingsData.filt2Minute;
      payload["filter2DurationHour"] = spaFilterSettingsData.filt2DurationHour;
      payload["filter2DurationMin"] = spaFilterSettingsData.filt2DurationMinute;
    }
    else
    {
      payload["filter2"] = "Disabled";
    }
  }

  

  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

    // Serialize JSON to a string
    String output;
    serializeJson(doc, output);
  
    memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_deviceStatus(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(400);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "deviceStatus";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  if(spaConfigurationData.pump1 >= 1)
  {
    payload["jet1"] = getMapDescription(_SpaStatusData.pump1, pumpMap);
  }
  if(spaConfigurationData.pump2 >= 1)
  {
    payload["jet2"] = getMapDescription(_SpaStatusData.pump2, pumpMap);
  }
  if(spaConfigurationData.pump3 >= 1)
  {
    payload["jet3"] = getMapDescription(_SpaStatusData.pump3, pumpMap);
  }
  if(spaConfigurationData.pump4 >= 1)
  {
    payload["jet4"] = getMapDescription(_SpaStatusData.pump4, pumpMap);
  }
  if(spaConfigurationData.blower >= 1)
  {
    payload["blower1"] = getMapDescription(_SpaStatusData.blower, onOffMap);
  }
  if(spaConfigurationData.light1 >= 1)
  {
    payload["light1"] = getMapDescription(_SpaStatusData.light1, onOffMap);
  }
  if(spaConfigurationData.light2 >= 1)
  {
    payload["light2"] = getMapDescription(_SpaStatusData.light2, onOffMap);
  }

  // payload["jet1"] = getMapDescription(_SpaStatusData.pump1, pumpMap);
  // payload["jet2"] = getMapDescription(_SpaStatusData.pump2, pumpMap);
  // payload["jet3"] = getMapDescription(_SpaStatusData.pump3, pumpMap);
  // payload["jet4"] = getMapDescription(_SpaStatusData.pump4, pumpMap);
  // payload["blower1"] = getMapDescription(_SpaStatusData.blower, onOffMap);
  // payload["light1"] = getMapDescription(_SpaStatusData.light1, onOffMap);
  // char currenttemp[20];
  // char settemp[20];
  // if(spaStatusData.tempScale == 0)
  // {
  //   snprintf(currenttemp, sizeof(currenttemp), "%3.1f Fahrenheit", _SpaStatusData.currentTemp);
  //   snprintf(settemp, sizeof(settemp), "%3.1f Fahrenheit", _SpaStatusData.setTemp);
  // }
  // else if(spaStatusData.tempScale == 1)
  // {
  //   snprintf(currenttemp, sizeof(currenttemp), "%3.1f Celsius", _SpaStatusData.currentTemp);
  //   snprintf(settemp, sizeof(settemp), "%3.1f Celsius", _SpaStatusData.setTemp);
  // }

  // payload["currentTemp"] = currenttemp;
  // payload["setTemp"] = settemp;
  payload["currentTemp"] = _SpaStatusData.currentTemp;
  payload["setTemp"] = _SpaStatusData.setTemp;
  payload["heatMode"] = getMapDescription(_SpaStatusData.heatingMode, heatingModeMap);
  payload["tempRange"] = getMapDescription(_SpaStatusData.tempRange, tempRangeMap);
  payload["heatStatus"] = getMapDescription(_SpaStatusData.heatingState, heatingStateMap);
  // if(_SpaStatusData.filterMode == 0)
  // {
  //   payload["filterCycle1"] = "Off";
  //   payload["filterCycle2"] = "Off";
  // }
  // else if(_SpaStatusData.filterMode == 1)
  // {
  //   payload["filterCycle1"] = "On";
  //   payload["filterCycle2"] = "Off";
  // }
  // else if(_SpaStatusData.filterMode == 2)
  // {
  //   payload["filterCycle1"] = "Off";
  //   payload["filterCycle2"] = "On";
  // }
  // else if(_SpaStatusData.filterMode == 3)
  // {
  //   payload["filterCycle1"] = "On";
  //   payload["filterCycle2"] = "On";
  // }

  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_currentTemp(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "currentTemp";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  // char currenttemp[20];
  
  // if(spaStatusData.tempScale == 0)
  // {
  //   snprintf(currenttemp, sizeof(currenttemp), "%3.1f Fahrenheit", _SpaStatusData.currentTemp);
  // }
  // else if(spaStatusData.tempScale == 1)
  // {
  //   snprintf(currenttemp, sizeof(currenttemp), "%3.1f Celsius", _SpaStatusData.currentTemp);
  // }

  // payload["currentTemp"] = currenttemp;


  
  payload["currentTemp"] = _SpaStatusData.currentTemp;
  
  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_setTemp(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "setTemp";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");

  // char settemp[20];
  // if(spaStatusData.tempScale == 0)
  // {
  //   snprintf(settemp, sizeof(settemp), "%3.1f Fahrenheit", _SpaStatusData.setTemp);
  // }
  // else if(spaStatusData.tempScale == 1)
  // {
  //   snprintf(settemp, sizeof(settemp), "%3.1f Celsius", _SpaStatusData.setTemp);
  // }

  // payload["setTemp"] = settemp;

  payload["setTemp"] = _SpaStatusData.setTemp;

  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_heatMode(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "heatMode";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  if(_SpaStatusData.heatingMode == 0x00)
  {
    payload["heatMode"] = "ready";
  }
  else if(_SpaStatusData.heatingMode == 0x01)
  {
    payload["heatMode"] = "rest";
  }
  else if(_SpaStatusData.heatingMode == 0x02)
  {
    payload["heatMode"] = "ready in rest";
  }

  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_setupInfo(SpaInformationData spa_information_data, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(350);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "setupInfo";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  
  // payload["setupNumber"] = spaInformationData.setupNumber;
  payload["Jet1"] = spaConfigurationData.pump1;
  payload["Jet2"] = spaConfigurationData.pump2;
  payload["Jet3"] = spaConfigurationData.pump3;
  payload["Jet4"] = spaConfigurationData.pump4;
  payload["Jet5"] = spaConfigurationData.pump5;
  payload["Jet6"] = spaConfigurationData.pump6;
  payload["Blower1"] = spaConfigurationData.blower;
  payload["Circulation_Pump"] = spaConfigurationData.circulationPump;
  payload["Light1"] = spaConfigurationData.light1;
  payload["Light2"] = spaConfigurationData.light2;
  payload["Aux1"] = spaConfigurationData.aux1;
  payload["Aux2"] = spaConfigurationData.aux2;
  payload["Mister"] = spaConfigurationData.mister;

  
  // payload["DIPSwitch"] = spaInformationData.dipSwitch;
   if(k == 0)
  {
    Log.notice("Cofig ::: Pump1 : %d, Pump2 : %d, Pump3 : %d\n \
       Pump4 : %d, Pump5 : %d, Pump6 : %d\n \
       Blower : %d, Circulation : %d\n \
       Light1 : %d, Light2 : %d\n \
       Aux1 : %d, Aux2 : %d, Mister : %d\n" \
       , spaConfigurationData.pump1, spaConfigurationData.pump2, spaConfigurationData.pump3, spaConfigurationData.pump4, \
       spaConfigurationData.pump5, spaConfigurationData.pump6, spaConfigurationData.blower, spaConfigurationData.circulationPump, \
       spaConfigurationData.light1, spaConfigurationData.light2, spaConfigurationData.aux1, spaConfigurationData.aux2, spaConfigurationData.mister);
    k = 1;
  }
  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_tempRange(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "tempRange";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  if(_SpaStatusData.tempRange == 1)
  {
    payload["tempRange"] = "High";
  }
  else if(_SpaStatusData.tempRange == 0)
  {
    payload["tempRange"] = "Low";
  }
  
  if(spaControlStatus.device_info)
  {
    spaControl_appand_device_info(&doc);
  }

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_bootupPacket(char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "bootupPacket";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");

  /* mac */
  char macAddr[18];  
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(macAddr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  payload["mac"] = macAddr;
  payload["fw_rev"] = FW_REV_STR;

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));

  Log.notice("Bootup Package created\n");
}

void spaControl_create_errorCode_message(char *json_str, uint8_t initMode, uint8_t reminderType)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  /* mac */
  char macAddr[18];  
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(macAddr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  doc["mac_address"] = macAddr;
  uint8_t errorCode = getMapDescription_uint8(reminderType, reminderTypeToerrorCodeMap);
  doc["error_code"] = getMapDescription(errorCode, errorCodeMap);
  doc["description"] = getMapDescription(reminderType, reminderTypeMap);

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));

  Log.notice("error code message created\n");
}


// Test:::
void switchTempRange(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x50);
  dataBuffer.push(0x00);
  // dataBuffer.push(0x32); // 08 10 BF 05 04 08 00 - Config request doesn't seem to work

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
  // Log.verbose(F("[rs485]: Sent Existing Client Response" CR), msgToString(dataBuffer).c_str()); // Commented for Test;
}

// Test:::
void switchHeatMode(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x51);
  dataBuffer.push(0x00);
  // dataBuffer.push(0x32); // 08 10 BF 05 04 08 00 - Config request doesn't seem to work

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
  // Log.verbose(F("[rs485]: Sent Existing Client Response" CR), msgToString(dataBuffer).c_str()); // Commented for Test;
}

void toggleJet1(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(WIFI_MODULE_ID);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x04);
  dataBuffer.push(0x00);
  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void toggleJet2(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(WIFI_MODULE_ID);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x05);
  dataBuffer.push(0x00);
  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void toggleJet3(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(WIFI_MODULE_ID);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x06);
  dataBuffer.push(0x00);
  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void toggleJet4(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(WIFI_MODULE_ID);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x07);
  dataBuffer.push(0x00);
  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void toggleBlower1(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(WIFI_MODULE_ID);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x0C);
  dataBuffer.push(0x00);
  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void toggleLight1(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(WIFI_MODULE_ID);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x11);
  dataBuffer.push(0x00);
  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void setTemp(float temp)
{
  if(spaStatusData.tempScale)
  {
   temp = temp*2;
  }
  else if(spaStatusData.tempScale == 0)
  {
    temp = temp;
  }
  else
  {
    temp = temp;
  }

  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x20);
  dataBuffer.push(temp);
  dataBuffer.push(0x00);
  // dataBuffer.push(0x32); // 08 10 BF 05 04 08 00 - Config request doesn't seem to work

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
  // Log.verbose(F("[rs485]: Sent Existing Client Response" CR), msgToString(dataBuffer).c_str()); // Commented for Test;
}

void filterCycleTrial(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x23);

  dataBuffer.push(spaFilterSettingsData.filt1Hour); // Filter 1 starting hours 23
  dataBuffer.push(spaFilterSettingsData.filt1Minute); // Filter 1 starting minutes 23
  dataBuffer.push(spaFilterSettingsData.filt1DurationHour); // Filter 1 Duration hours 23
  dataBuffer.push(spaFilterSettingsData.filt1DurationMinute); // Filter 1 Duration hours 23

  // if(filterCycleData->filter2StartHour != 0)
  // {
  //   Log.notice("filter 2 enable\n");
  //   filterCycleData->filter2StartHour = filterCycleData->filter2StartHour | (1 << 7);
  // }
  // else
  // {
  //   Log.notice("filter 2 nana\n");
  //   filterCycleData->filter2StartHour = 0;
  // }
  if(spaFilterSettingsData.filt2Enable)
  {
    spaFilterSettingsData.filt2Hour =spaFilterSettingsData.filt2Hour | (1 << 7);
    dataBuffer.push(spaFilterSettingsData.filt2Hour); // Filter 2 enable(bit 7) & stating hours 23
    dataBuffer.push(spaFilterSettingsData.filt2Minute); // Filter 2 minutes 23
    dataBuffer.push(spaFilterSettingsData.filt2DurationHour); // Filter 2 Duration hours 23
    dataBuffer.push(spaFilterSettingsData.filt2DurationMinute); // Filter 2 Duration hours 23

  }
  else
  {
    dataBuffer.push(0); // Filter 2 enable(bit 7) & stating hours 23
    dataBuffer.push(0); // Filter 2 minutes 23
    dataBuffer.push(0); // Filter 2 Duration hours 23
    dataBuffer.push(0); // Filter 2 Duration hours 23
  }
  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void informationRequest(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x22);
  dataBuffer.push(0x02);
  dataBuffer.push(0x00);
  dataBuffer.push(0x00);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}


void configRequest(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x22);
  dataBuffer.push(0x00);
  dataBuffer.push(0x00);
  dataBuffer.push(0x01);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
  
  // spaControlStatus.setupInfo = true;
}


void toggleHoldState(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x11);
  dataBuffer.push(0x3C);
  dataBuffer.push(0x00);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}


void setTime(int hour, int minute)
{
  if(spaStatusData.clockMode)
  {
    hour |= (1 << 7);
  }
  else if(spaStatusData.clockMode = 0)
  {
    hour = hour;
  }
  
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x21);
  dataBuffer.push(hour);
  dataBuffer.push(minute);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void setCleanupCycle(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x27);
  dataBuffer.push(0x03);
  dataBuffer.push(cleanupCycleTime);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void setClockMode(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x27);
  dataBuffer.push(0x02);
  dataBuffer.push(spaStatusData.clockMode);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void setTempScale(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x27);
  dataBuffer.push(0x01);
  dataBuffer.push(spaStatusData.tempScale);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}

void setM8(void)
{
  CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
  dataBuffer.push(id);
  dataBuffer.push(0xBF);
  dataBuffer.push(0x27);
  dataBuffer.push(0x06);
  dataBuffer.push(m8);

  addCRC(dataBuffer);
  sendMessageToSpa(dataBuffer);
}