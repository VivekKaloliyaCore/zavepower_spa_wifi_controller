#include "wifiModule.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoLog.h>
#include <time.h>

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <restartReason.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

#include "wireless_debugger.h"

Preferences preferences;
// AsyncWebServer server(80);
#include "../httpServer/httpServer.h"

WiFiManager wifiManager;
char gatewayName[20];

static int wifi_sta_connect_retry = 0;
bool ap_configuration_on = false;
bool restart_esp = false;

unsigned long lastCheck = 0;
bool internetOK = false;
IPAddress ip;

char g_utc_offset[8]    = {0};
char g_timezone[32]     = {0};

void wifiModuleCnnectToWiFi();
bool wifiModuleCnnectToWiFiOneTime(void);

void wifiModuleStartAPMode()
{
  set_wireless_debugger_is_spa_configured(false);
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
        // configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
        Log.notice(F("[WiFi]: Connected, IP Address: %s" CR), WiFi.localIP().toString().c_str());

        set_wireless_debugger_wifi_reconn_counter_up();

        syncTimeFromIpApi();

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

        /* sync time */
        struct tm time_now = getStructTime();
        syncWithNetworkTime(time_now.tm_hour, time_now.tm_min);

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
    set_wireless_debugger_is_spa_configured(true);
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

  // if (millis() - lastCheck > 10000) {  // every 10s
  //     lastCheck = millis();
  //     internetOK = WiFi.hostByName("google.com", ip);
  //     Log.notice("Internet is %s\n", internetOK?"Ok.":"Not ok.");
  // }

  // if (millis() - lastCheck > 5000) {  // every 10s
  //     lastCheck = millis();
  //     struct tm t;
  //     getLocalTime(&t);
  //     Serial.println(&t, "%Y-%m-%d %H:%M:%S");
  // }
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

struct tm getStructTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Log.error(F("[WiFi]: Obtaining Time failed" CR));
  }

  return timeinfo;
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

int32_t wifiModuleGetRSSI(void)
{
  return WiFi.RSSI();
}

const char* get_timezone(void)
{
    return g_timezone;
}
 
const char* get_utc_offset(void)
{
    return g_utc_offset;
}
 
bool set_timezone(const char* tz)
{
    if (!tz) return false;
 
    strlcpy(g_timezone, tz, sizeof(g_timezone));
    return true;
}
 
bool set_utc_offset(const char* offset)
{
    if (!offset || strlen(offset) != 5) return false;
 
    strlcpy(g_utc_offset, offset, sizeof(g_utc_offset));
    return true;
}

bool syncTimeFromIpApi(void)
{
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure(); // skip cert validation

    HTTPClient https;
    if (!https.begin(client, "https://ipapi.co/json")) {
        return false;
    }

    int httpCode = https.GET();
    if (httpCode != 200) {
        https.end();
        return false;
    }

    String payload = https.getString();
    https.end();

    StaticJsonDocument<1024> doc;
    if (deserializeJson(doc, payload) != DeserializationError::Ok) {
        return false;
    }

    const char* json_timezone = doc["timezone"]; // "Asia/Kolkata"
    if (!json_timezone || strlen(json_timezone) == 0) {
        return false;
    }

    const char* json_utc_offset = doc["utc_offset"]; // "+0530"
    if (!json_utc_offset || strlen(json_utc_offset) != 5) {
        return false;
    }

    set_timezone(json_timezone);
    set_utc_offset(json_utc_offset);
    Log.noticeln("Stored UTC Offset : %s", get_utc_offset());
    Log.noticeln("Stored Timezone   : %s", get_timezone());

    // ---- Convert "+0530" → POSIX TZ ----
    int sign = (json_utc_offset[0] == '-') ? -1 : 1;
    int hours = (json_utc_offset[1] - '0') * 10 + (json_utc_offset[2] - '0');
    int minutes = (json_utc_offset[3] - '0') * 10 + (json_utc_offset[4] - '0');

    int tz_hours = -sign * hours;

    char tz[20];
    if (minutes == 0) {
        snprintf(tz, sizeof(tz), "UTC%+d", tz_hours);
    } else {
        snprintf(tz, sizeof(tz), "UTC%+d:%02d", tz_hours, minutes);
    }
    Log.noticeln("Setting TZ to: %s", tz);   // LOC-5:30 for India

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      delay(500);
      Log.notice(".");
    }
    // Print UTC
    Log.notice("UTC: %d:%d:%d\n",
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    // setenv("TZ", "IST-5:30", 1);  // ✅ Set IST timezone
    // setenv("TZ", "UTC-5:30", 1);  // ✅ Set IST timezone
    setenv("TZ", tz, 1);  // ✅ Set IST timezone
    tzset();
    while (!getLocalTime(&timeinfo)) {
      delay(500);
      Log.notice(".");
    }
    Log.notice("Local time: %d:%d:%d\n",
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Log.noticeln("Time synchronized (IST)");
    Log.noticeln("");

    (strstr(getStringTime().c_str(), "Failed to obtain time")) ? false : true;

    return true;
}