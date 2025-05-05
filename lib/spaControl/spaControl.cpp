#include <Arduino.h>
#include <CircularBuffer.hpp>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <WiFi.h>

#include  <spaUtilities.h>
#include <spaMessage.h>

#include "spaControl.h"
#include "rs485.h"
#include "spaMqttMessage.h"
#include <wifiModule.h>
#include "../../src/config.h"
#include "../../src/main.h"
#include <balboa.h>

//Create the second JSON document
DynamicJsonDocument deviceInfoCopy(512);

mqtt_params_t mqtt_params = {0};

// Global variable to send temp to set.
float sendSetTemp = 0;

Ticker spaCmdSendTimer;
bool spaCmdSendTimerRunning = false;
bool sendSpaCmdSend = false;

static spaControlParams_t spaControlParams = {0};
static spaControlStatus_t spaControlStatus = {0};

filterCycleData_t filterCycleData = {0};
// SpaFilterSettingsData spaFilterSettingsData = {0};

// Local Function
void switchTempRange(void);
void switchHeatMode(void);
void toggleJet1(void);
void toggleJet2(void);
void toggleLight1(void);
void toggleBlower1(void);
void setTemp(float temp);
void spaControl_appand_device_info(StaticJsonDocument<200> *doc);
void spaControl_create_filter_cycle(char *json_str);

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
}

spaControlParams_t get_spaControlParams(void)
{
  return spaControlParams;
}

void set_spaControlParams(spaControlParams_t _spaControlParams)
{
  spaControlParams.is_jet1_present = _spaControlParams.is_jet1_present;
  spaControlParams.jet1 = _spaControlParams.jet1;

  spaControlParams.is_jet2_present = _spaControlParams.is_jet2_present;
  spaControlParams.jet2 = _spaControlParams.jet2;

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
  // spaControlParams.is_reset_wifi_sta_present = _spaControlParams.is_reset_wifi_sta_present;
  // spaControlParams.reset_wifi_sta = _spaControlParams.reset_wifi_sta;
}

spaControlStatus_t get_spaControlStatus(void)
{
  return spaControlStatus;
}

void set_spaControlStatus(spaControlStatus_t _spaControlStatus)
{
  spaControlStatus.deviceStatus = _spaControlStatus.deviceStatus;
  spaControlStatus.bootupPacket = _spaControlStatus.bootupPacket;
  spaControlStatus.currentTemp = _spaControlStatus.currentTemp;
  spaControlStatus.setTemp = _spaControlStatus.setTemp;
  spaControlStatus.tempRange = _spaControlStatus.tempRange;
  spaControlStatus.heatMode = _spaControlStatus.heatMode;
  spaControlStatus.device_info = _spaControlStatus.device_info;
  spaControlStatus.device_id = _spaControlStatus.device_id;
  spaControlStatus.user_id = _spaControlStatus.user_id;
  spaControlStatus.filterCycle = _spaControlStatus.filterCycle;
  spaControlStatus.filter1 = _spaControlStatus.filter1;
  spaControlStatus.filter2 = _spaControlStatus.filter2;
}


void spaControl_action(void)
{
  // if(spaControlParams.setTempCommand)
  // {
  //   spaControlParams.setTempCommand = false;
  //   setTemp(0x1E);
  // }


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
      spaControlParams.is_filterCycle_present = false;
      filterCycleTrial();
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



void spaControl_mqtt_action(void)
{

  // Log.notice("SpaControl_mqtt_action!!!");

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
    spaControl_create_filter_cycle(json_str);

    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
  }

  if(spaControlStatus.bootupPacket)
  {
    spaControlStatus.bootupPacket = false;
    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    // SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_bootupPacket(json_str);

    spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], json_str, strlen(json_str));
    Log.notice("Boot Up pacage Published\n");
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
          if(sendSetTemp >= 26.5 && sendSetTemp <= 40)
          {
            spaControlParams->is_set_temp_present = true;
            spaControlParams->setTempCommand = true;
          }
        }
        else if(spaStatusData.tempRange == 0) // Set temp range is LOW. i.e. : 10 degree celcius to 37 degree celcius
        {
          if(sendSetTemp >= 10 && sendSetTemp <= 37)
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
      else if(doc["payload"].containsKey("blower1"))
      {
        int blower1 = doc["payload"]["blower1"];
        Log.notice("Blower1: %d\n", blower1);

        spaControlParams->is_blower1_present = true;
        spaControlParams->blower1 = blower1;
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

      if(doc["payload"].containsKey("filterCycle"))
      {
        spaControlParams->is_filterCycle_present = true;
        if(doc["payload"]["filterCycle"].containsKey("1"))
        {
          spaFilterSettingsData.filt1Hour = doc["payload"]["filterCycle"]["1"]["startTimeHr"];
          spaFilterSettingsData.filt1Minute = doc["payload"]["filterCycle"]["1"]["startTimeMin"];
          spaFilterSettingsData.filt1DurationHour = doc["payload"]["filterCycle"]["1"]["durationHr"];
          spaFilterSettingsData.filt1DurationMinute = doc["payload"]["filterCycle"]["1"]["durationMin"];

          // filterCycleData.filter1StartHour = doc["payload"]["filterCycle"]["1"]["startTimeHr"];
          // filterCycleData.filter1StartMinute = doc["payload"]["filterCycle"]["1"]["startTimeMin"];
          // filterCycleData.filter1DurationHour = doc["payload"]["filterCycle"]["1"]["durationHr"];
          // filterCycleData.filter1DurationMinute = doc["payload"]["filterCycle"]["1"]["durationMin"];
        }
        if(doc["payload"]["filterCycle"].containsKey("2"))
        {

          Log.notice("Payload 2 Received\n");
          spaFilterSettingsData.filt2Enable = 1;
          spaFilterSettingsData.filt2Hour = doc["payload"]["filterCycle"]["2"]["startTimeHr"];
          spaFilterSettingsData.filt2Minute = doc["payload"]["filterCycle"]["2"]["startTimeMin"];
          spaFilterSettingsData.filt2DurationHour = doc["payload"]["filterCycle"]["2"]["durationHr"];
          spaFilterSettingsData.filt2DurationMinute = doc["payload"]["filterCycle"]["2"]["durationMin"];
          // filterCycleData.filter2StartHour = doc["payload"]["filterCycle"]["2"]["startTimeHr"];
          // filterCycleData.filter2StartMinute = doc["payload"]["filterCycle"]["2"]["startTimeMin"];
          // filterCycleData.filter2DurationHour = doc["payload"]["filterCycle"]["2"]["durationHr"];
          // filterCycleData.filter2DurationMinute = doc["payload"]["filterCycle"]["2"]["durationMin"];
        }
        else
        {
          Log.notice("Payload 2 Not Received\n");
          spaFilterSettingsData.filt2Enable = 0;
          spaFilterSettingsData.filt2Hour = 0;
          spaFilterSettingsData.filt2Minute = 0;
          spaFilterSettingsData.filt2DurationHour = 0;
          spaFilterSettingsData.filt2DurationMinute = 0;
        }
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
      else if(doc["payload"].containsKey("filterCycle"))
      {
        spaControlStatus->filterCycle = true;
        if(doc["payload"]["filterCycle"].containsKey("cycle1"))
        {
          spaControlStatus->filter1 = true;
        }
        if(doc["payload"]["filterCycle"].containsKey("cycle2"))
        {
          spaControlStatus->filter2 = true;
        }
      }
    }
    if(doc.containsKey("device_info"))
    {
      // Log.notice("hello\n");
      spaControlStatus->device_info = true;
      deviceInfoCopy.set(doc["device_info"]);  // Deep copy


      // if(doc["device_info"].containsKey("user_id"))
      // {
      //   // Log.notice("hehe\n");
      //   spaControlStatus->device_info = true;
      //   spaControlStatus->device_id = doc["device_info"]["device_id"];
      //   spaControlStatus->user_id = doc["device_info"]["user_id"];
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

void spaControl_appand_device_info(DynamicJsonDocument *doc)
{
  spaControlStatus.device_info = false;

  // JsonArray device_info = *doc->[device_info];
  // device_info.add(deviceInfoCopy);
  
  // JsonObject device_info = &doc->set(deviceInfoCopy);
  
  // JsonObject device_info = doc->createNestedObject("device_info");
  // device_info["device_id"] = spaControlStatus.device_id;
  // device_info["user_id"] = spaControlStatus.user_id;
}

void spaControl_create_filter_cycle(char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "filterCycle";

  JsonObject payload = doc.createNestedObject("payload");
    // spaControlStatus.filter1 = false;
    payload["filter1StartHour"] = spaFilterSettingsData.filt1Hour;
    payload["filter1StartMin"] = spaFilterSettingsData.filt1Minute;
    payload["filter1DurationHour"] = spaFilterSettingsData.filt1DurationHour;
    payload["filter1DurationHour"] = spaFilterSettingsData.filt1DurationMinute;

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
  DynamicJsonDocument doc(300);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "deviceStatus";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  payload["jet1"] = getMapDescription(_SpaStatusData.pump1, pumpMap);
  payload["jet2"] = getMapDescription(_SpaStatusData.pump2, pumpMap);
  payload["blower1"] = getMapDescription(_SpaStatusData.blower, onOffMap);
  payload["light1"] = getMapDescription(_SpaStatusData.light1, onOffMap);
  payload["heatingMode"] = getMapDescription(_SpaStatusData.heatingMode, heatingModeMap);
  payload["tempRange"] = getMapDescription(_SpaStatusData.tempRange, tempRangeMap);
  payload["currentTemp"] = _SpaStatusData.currentTemp;
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

void spaControl_create_currentTemp(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "currentTemp";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
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
  doc["msgT"] = "setTemp";

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

void spaControl_create_tempRange(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  DynamicJsonDocument doc(200);

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "setTemp";

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
  temp = temp*2;
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
  dataBuffer.push(spaFilterSettingsData.filt2DurationHour); // Filter 1 Duration hours 23
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