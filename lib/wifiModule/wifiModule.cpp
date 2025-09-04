#include "wifiModule.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoLog.h>
#include <time.h>

#include <restartReason.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

Preferences preferences;
// AsyncWebServer server(80);
#include "../httpServer/httpServer.h"

WiFiManager wifiManager;
char gatewayName[20];

static int wifi_sta_connect_retry = 0;
bool ap_configuration_on = false;
bool restart_esp = false;

void wifiModuleCnnectToWiFi();
bool wifiModuleCnnectToWiFiOneTime(void);

void wifiModuleStartAPMode()
{
  apServerOn();
}

void wifiModuleCnnectToWiFi()
{
  // if(WiFi.getMode() != WIFI_MODE_STA)
  // if( (WiFi.getMode() != WIFI_MODE_STA) && (WiFi.getMode() != WIFI_MODE_APSTA))
  if( (WiFi.getMode() == WIFI_MODE_AP) || (ap_configuration_on) )
    return;

  String savedSSID = preferences.getString("ssid", "");
  String savedPass = preferences.getString("pass", "");

  // if (savedSSID.length() > 0)
  if ( (savedSSID.length() > 0) && (wifi_sta_connect_retry < WIFI_STA_CONNECT_RETRIES) )
  {
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    Log.notice("[WiFi]: Connecting to Wi-Fi [%s]\n", savedSSID.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        attempts++;
        yield();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
        Log.notice(F("[WiFi]: Connected, IP Address: %s" CR), WiFi.localIP().toString().c_str());

        int timeSyncRetry = 0;
        while(strstr(getStringTime().c_str(), "Failed to obtain time") && timeSyncRetry < 3)
        {
          delay(1000);
          timeSyncRetry++;
        }
        if(timeSyncRetry >= 3)
        {
          ESP.restart();
        }

        Log.notice(F("[WiFi]: Time: %s" CR), getStringTime().c_str());
        // otaSetup();

        wifi_sta_connect_retry = 0;
    }
    else
    {
      Log.notice("[WiFi]: Failed to connect to Wi-Fi\n");
      wifi_sta_connect_retry++;
    }
  }
  // else if( (strlen(WIFI_SSID) > 0) && (wifi_sta_connect_retry < WIFI_STA_CONNECT_RETRIES) )
  // {
  //   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //   Log.notice("[WiFi]: Setup hotspot: [ssid: %s] | [pass: %s]\n", WIFI_SSID, WIFI_PASSWORD);
  //   Log.notice("[WiFi]: Connecting to Wi-Fi [%s]\n", WIFI_SSID);
  //   int attempts = 0;
  //   while (WiFi.status() != WL_CONNECTED && attempts < 20)
  //   {
  //       delay(500);
  //       attempts++;
  //       yield();
  //   }

  //   if (WiFi.status() == WL_CONNECTED)
  //   {
  //       configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
  //       Log.notice(F("[WiFi]: Connected, IP Address: %s" CR), WiFi.localIP().toString().c_str());

  //       int timeSyncRetry = 0;
  //       while(strstr(getStringTime().c_str(), "Failed to obtain time") && timeSyncRetry < 3)
  //       {
  //         delay(1000);
  //         timeSyncRetry++;
  //       }
  //       if(timeSyncRetry >= 3)
  //       {
  //         ESP.restart();
  //       }

  //       Log.notice(F("[WiFi]: Time: %s" CR), getStringTime().c_str());
  //       // otaSetup();

  //       wifi_sta_connect_retry = 0;
  //   }
  //   else
  //   {
  //     Log.notice("[WiFi]: Failed to connect to Wi-Fi\n");
  //     wifi_sta_connect_retry++;
  //   }
  // }
  else
  {
    if(wifi_sta_connect_retry >= WIFI_STA_CONNECT_RETRIES)
    {
      wifi_sta_connect_retry = 0;

      Log.notice("[WiFi]: Re-configure the Wi-Fi\n");

      wifiModuleStartAPMode();   // Always keep AP active
    }
  }
}

void wifiModuleSetup()
{
  String s = WiFi.macAddress();
  sprintf(gatewayName, "spa-%.2s%.2s%.2s%.2s%.2s%.2s", s.c_str(),
          s.c_str() + 3, s.c_str() + 6, s.c_str() + 9, s.c_str() + 12,
          s.c_str() + 15);

  WiFi.setTxPower(WIFI_POWER_19_5dBm); // this sets wifi to highest power
  WiFi.setHostname(gatewayName);
  Log.notice(F("[WiFi]: Hostname: %s" CR), WiFi.getHostname());

  preferences.begin("wifi", false);

  String savedSSID = preferences.getString("ssid", "");
  if(savedSSID.length() <= 0)
  {
    // ap_configuration_on = true;
    wifiModuleStartAPMode();
  }
  else
  {
    ap_configuration_on = false;
    WiFi.mode(WIFI_STA);
  }

  String user_id = preferences.getString("user_id", "");
  Log.notice(F("[WiFi]: User ID: %s" CR), user_id.c_str());

  api.begin("myapi", true);
  String url = api.getString("apiurl", "");
  if (url == "")
  {
    api.end();
    Log.notice("><><><>><>>> it was null\n");
    api.begin("myapi", false);
    api.putString("apiurl", "https://api.zavepower.cloud/api/device/error-logs-mac-address");
    api.end();
  }
  else
  {
    Log.notice("><><><>><>>> Was not null!!!\n");
    api.end();
  }
}

void wifiModuleLoop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    // wifiConnect();
    wifiModuleCnnectToWiFi();
  }

  if(restart_esp)
  {
    restart_esp = false;
    delay(2000);
    ESP.restart();
  }

  if(flag_server_on != 1)
  {
    stServerOn();
  }
}

/* Functions */

void wifiConnect()
{
  Log.notice(F("[WiFi]: Connecting to %s" CR), WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long timeout = millis() + WIFI_CONNECT_TIMEOUT;

  while (WiFi.status() != WL_CONNECTED && millis() < timeout)
  {
    yield();
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Log.error(F("[WiFi]: Connect failed to %s" CR), WIFI_SSID);
  }
  else
  {
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
    Log.notice(F("[WiFi]: Connected, IP Address: %s" CR), WiFi.localIP().toString().c_str());

    int timeSyncRetry = 0;
    while(strstr(getStringTime().c_str(), "Failed to obtain time") && timeSyncRetry < 3)
    {
      delay(1000);
      timeSyncRetry++;
    }
    if(timeSyncRetry >= 3)
    {
      ESP.restart();
    }

    Log.notice(F("[WiFi]: Time: %s" CR), getStringTime().c_str());
    // otaSetup();
  }
}

String getStringTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Log.error(F("[WiFi]: Obtaining Time failed" CR));
    return String("Failed to obtain time");
  }
  char timeCharArray[64];
  strftime(timeCharArray, sizeof(timeCharArray), "%Y-%m-%d %H:%M:%S", &timeinfo);

  return String(timeCharArray);
}

void notifyOfUpdateStarted()
{
  Log.notice(F("[WiFi]: Arduino OTA Update Start" CR));
}

void notifyOfUpdateEnded()
{
  Log.notice(F("[WiFi]: Arduino OTA Update Complete" CR));
  setLastRestartReason("OTA Update");
}

void wifiModuleEraseStaConfig(void)
{
  preferences.remove("ssid");
  preferences.remove("pass");
}

bool wifiModuleCnnectToWiFiOneTime(void)
{
  if(WiFi.getMode() == WIFI_MODE_AP)
    return false;

  String savedSSID = preferences.getString("ssid", "");
  String savedPass = preferences.getString("pass", "");

  if (savedSSID.length() > 0)
  {
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    Log.notice("[WiFi]: Connecting to Wi-Fi [%s]\n", savedSSID.c_str());
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        attempts++;
        yield();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
        Log.notice(F("[WiFi]: Connected, IP Address: %s" CR), WiFi.localIP().toString().c_str());

        WiFi.disconnect();

        return true;
    }
    else
    {
      Log.notice("[WiFi]: Failed to connect to Wi-Fi\n");
    }
  }

  return false;
}
