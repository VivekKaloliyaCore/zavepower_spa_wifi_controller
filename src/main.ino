#include <Arduino.h>
#include <WiFiManager.h>
#include <ArduinoLog.h>

// Local Libraries
#include <restartReason.h>
#include <wifiModule.h>
#include <spaMessage.h>
#include <spaUtilities.h>
#include <mqttModule.h>
#include <rs485.h>
#include <bridge.h>

#include "main.h"
#include "../httpsClient/httpsClient.h"

String buildDefinitionString = "";
#define addBuildDefinition(name) buildDefinitionString += #name " ";

void setup()
{
  // Launch serial for debugging purposes
  Serial.begin(SERIAL_BAUD);
  delay(2000);
  Log.setPrefix(logPrintPrefix);
  Log.begin(LOG_LEVEL, &Serial);
  logSection("WELCOME TO esp32_balboa_spa");
  logSection("Build Definitions");
  Log.notice(F("Version: %s" CR), VERSION);
  Log.notice(F("Build: %s" CR), BUILD);

#ifdef ESP32S3
  Log.notice(F("Build for ESP32S3" CR));
#else
  Log.notice(F("Build for ESP32" CR));
#endif

#ifdef ARDUINO_ESP32S3_DEV
  Log.notice(F("Build for ARDUINO_ESP32S3_DEV" CR));
#endif

  Log.notice(F("Build Definitions: %s" CR), buildDefinitionString.c_str());

  logSection("ESP Information");
  Log.notice(F("Last restart reason: %s" CR), getLastRestartReason().c_str());
  Log.verbose(F("Free heap: %d bytes" CR), ESP.getFreeHeap());
  Log.verbose(F("Free sketch space: %d bytes" CR), ESP.getFreeSketchSpace());
  Log.verbose(F("Chip ID: %x" CR), ESP.getEfuseMac());

  Log.verbose(F("Flash chip size: %d bytes" CR), ESP.getFlashChipSize());
  //  Log.verbose(F("Flash chip speed: %d Hz" CR), ESP.getFlashChipSpeed());
  Log.verbose(F("CPU frequency: %d Hz" CR), ESP.getCpuFreqMHz());
  Log.verbose(F("SDK version: %s" CR), ESP.getSdkVersion());

  logSection("Wifi Module Setup");
  wifiModuleSetup();
  logSection("MQTT Module Setup");
  mqttModuleSetup();
  logSection("RS485 Module Setup");
  rs485Setup();
  logSection("Spa Message Setup");
  spaMessageSetup();

  

  logSection("Setup Complete");
  logSection(FW_REV_STR);

  esp_log_level_set("*", ESP_LOG_DEBUG);
  
  // // logSection("This is UPDATED FIRMWARE....");


}

void loop()
{
  // rs485Loop();
  wifiModuleLoop();



  if (WiFi.status() == WL_CONNECTED)
  {
    rs485Loop();
    mqttModuleLoop();
    spaMessageLoop();
  }

}