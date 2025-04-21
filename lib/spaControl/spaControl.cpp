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

Ticker spaCmdSendTimer;
bool spaCmdSendTimerRunning = false;
bool sendSpaCmdSend = false;

static spaControlParams_t spaControlParams = {0};
static spaControlStatus_t spaControlStatus = {0};

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
}

void spaControl_action(void)
{
  if(spaControlParams.is_jet1_present)
  {
    if(spaControlParams.jet1)
    {
      if(sendSpaCmdSend || !spaCmdSendTimerRunning)
      {
        CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
        dataBuffer.push(WIFI_MODULE_ID);
        dataBuffer.push(0xBF);
        dataBuffer.push(0x11);
        dataBuffer.push(0x04);
        dataBuffer.push(0x00);
        addCRC(dataBuffer);
        sendMessageToSpa(dataBuffer);

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
        CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
        dataBuffer.push(WIFI_MODULE_ID);
        dataBuffer.push(0xBF);
        dataBuffer.push(0x11);
        dataBuffer.push(0x05);
        dataBuffer.push(0x00);
        addCRC(dataBuffer);
        sendMessageToSpa(dataBuffer);

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
        CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
        dataBuffer.push(WIFI_MODULE_ID);
        dataBuffer.push(0xBF);
        dataBuffer.push(0x11);
        dataBuffer.push(0x0C);
        dataBuffer.push(0x00);
        addCRC(dataBuffer);
        sendMessageToSpa(dataBuffer);

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
        CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> dataBuffer;
        dataBuffer.push(WIFI_MODULE_ID);
        dataBuffer.push(0xBF);
        dataBuffer.push(0x11);
        dataBuffer.push(0x11);
        dataBuffer.push(0x00);
        addCRC(dataBuffer);
        sendMessageToSpa(dataBuffer);

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
      if(doc["payload"].containsKey("jet1"))
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