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

// Global variable to send temp to set.
float sendSetTemp = 0;


Ticker spaCmdSendTimer;
bool spaCmdSendTimerRunning = false;
bool sendSpaCmdSend = false;

static spaControlParams_t spaControlParams = {0};
static spaControlStatus_t spaControlStatus = {0};


// Local Function
void switchTempRange(void);
void switchHeatMode(void);
void toggleJet1(void);
void toggleJet2(void);
void toggleLight1(void);
void toggleBlower1(void);
void setTemp(float temp);

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
  if(spaControlStatus.deviceStatus)
  {
    spaControlStatus.deviceStatus = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_deviceStatus(spa_status_data, json_str);

    spaMqttMessage_publish_message("response", json_str, strlen(json_str));
  }

  if(spaControlStatus.currentTemp)
  {
    spaControlStatus.currentTemp = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_currentTemp(spa_status_data, json_str);

    spaMqttMessage_publish_message("response", json_str, strlen(json_str));
  }

  if(spaControlStatus.setTemp)
  {
    spaControlStatus.setTemp = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_setTemp(spa_status_data, json_str);

    spaMqttMessage_publish_message("response", json_str, strlen(json_str));
  }

  if(spaControlStatus.heatMode)
  {
    spaControlStatus.heatMode = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_heatMode(spa_status_data, json_str);

    spaMqttMessage_publish_message("response", json_str, strlen(json_str));
  }

  if(spaControlStatus.tempRange)
  {
    spaControlStatus.tempRange = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_tempRange(spa_status_data, json_str);

    spaMqttMessage_publish_message("response", json_str, strlen(json_str));
  }

  if(spaControlStatus.bootupPacket)
  {
    spaControlStatus.bootupPacket = false;

    char json_str[512];
    memset(&json_str[0], 0, sizeof(json_str));
    SpaStatusData spa_status_data = spaMessage_get_spaStatusData();
    spaControl_create_bootupPacket(json_str);

    spaMqttMessage_publish_message("response", json_str, strlen(json_str));
  }
}

bool spaControl_parse_action_command(char *json_str, spaControlParams_t *spaControlParams, spaControlStatus_t *spaControlStatus, otaParams_t *otaParams)
{
  // Create a JSON document
  StaticJsonDocument<200> doc;

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

      // else if(doc["payload"].containsKey("setTemp"))
      // {
      //   int setTemp = doc["payload"]["setTemp"];
      //   Log.notice("Set Temprature : %d\n", setTemp);

      //   spaControlParams->setTempCommand = true;
      //   // spaControlParams->reset_wifi_sta = setTemp;
      // }

    }
    else
      return false;
  }
  /* action 'get' */
  else if(strstr(action, "get"))
  {
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
    }
    else
      return false;
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

void spaControl_create_deviceStatus(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  StaticJsonDocument<200> doc;

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

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_currentTemp(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  StaticJsonDocument<200> doc;

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "currentTemp";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  payload["currentTemp"] = _SpaStatusData.currentTemp;

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_setTemp(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  StaticJsonDocument<200> doc;

  // Add key-value pairs
  doc["action"] = "response";
  doc["msgT"] = "setTemp";

  // Create "payload" as a nested object
  JsonObject payload = doc.createNestedObject("payload");
  payload["setTemp"] = _SpaStatusData.setTemp;
  
  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_heatMode(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  StaticJsonDocument<200> doc;

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


  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_tempRange(SpaStatusData _SpaStatusData, char *json_str)
{
  // Create a JSON document
  StaticJsonDocument<200> doc;

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
  

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
}

void spaControl_create_bootupPacket(char *json_str)
{
  // Create a JSON document
  StaticJsonDocument<200> doc;

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

  // Serialize JSON to a string
  String output;
  serializeJson(doc, output);

  memcpy(json_str, output.c_str(), strlen(output.c_str()));
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