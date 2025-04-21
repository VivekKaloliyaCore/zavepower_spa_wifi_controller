#include "wifiModule.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <ArduinoLog.h>
#include <time.h>
#include <ArduinoJson.h>  // Include ArduinoJson library

#include <restartReason.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

Preferences preferences;
AsyncWebServer server(80);

WiFiManager wifiManager;
char gatewayName[20];

static int wifi_sta_connect_retry = 0;
static bool ap_configuration_on = false;
static bool restart_esp = false;

void wifiModuleCnnectToWiFi();
bool wifiModuleCnnectToWiFiOneTime(void);

void wifiModuleStartAPMode()
{
  ap_configuration_on = true;

  WiFi.mode(WIFI_AP);

  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  Log.notice("[WiFi]: AP Mode Started: [SSID: %s] | [Password: %s]\n", WIFI_AP_SSID, WIFI_AP_PASSWORD);
  Log.notice(F("[WiFi]: AP IP Address: %s" CR), WiFi.softAPIP().toString().c_str());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", htmlPage);
  });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
      // WiFi.mode(WIFI_AP_STA); // Ensure AP mode is enabled before scanning
      // delay(100); // Small delay for stability

      bool wasConnecting = (WiFi.status() != WL_CONNECTED); // Check if STA is trying to connect

      if (wasConnecting) {
          WiFi.disconnect(); // Stop STA mode before scanning
          delay(100); // Short delay for stability
      }
      
      int numNetworks = WiFi.scanNetworks();
      String json = "[";
      for (int i = 0; i < numNetworks; i++) {
          if (i) json += ",";
          json += "\"" + WiFi.SSID(i) + "\"";
      }
      json += "]";

      WiFi.scanDelete(); // Free memory after scanning

      if (wasConnecting) {
          WiFi.begin(); // Restart STA mode if it was previously connecting
      }

      request->send(200, "application/json", json);
  });

  server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (!request->hasParam("ssid") || !request->hasParam("pass")) {
          request->send(400, "text/plain", "Missing parameters");
          return;
      }

      String ssid = request->getParam("ssid")->value();
      String pass = request->getParam("pass")->value();
      preferences.putString("ssid", ssid);
      preferences.putString("pass", pass);

      request->send(200, "text/plain", "Saved successfully, Redirecting...");
      // delay(2000);
      // ESP.restart();
  });

  server.on("/wifiCheck", HTTP_GET, [](AsyncWebServerRequest *request) {
      // Perform Wi-Fi connection logic
      if (wifiModuleCnnectToWiFiOneTime()) {
          request->redirect("/success?success=1");
      } else {
          request->redirect("/success?success=0");
      }
  });

  server.on("/success", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (!request->hasParam("success")) return; // fallback safety

      String result = request->getParam("success")->value();

      String html = "<!DOCTYPE html><html><head>";  
      html +="<meta charset='UTF-8'><meta name='viewport'content='width=device-width, initial-scale=1.0'><title>Wi-Fi Status</title>";
      html += "<style>";
      html += "body{font-family:sans-serif;background:#0d2628;color:#fff;text-align:center;padding-top:50px}";
      html += ".wifi-setup-area{margin:0 auto;max-width:600px}";
      html += ".wifi-title{color:#1cbcc7}";
      html += ".msg{font-size:22px;margin-bottom:20px;display: flex;justify-content: center;flex-wrap: wrap;gap: 6px;align-items: flex-end;}";
      html += "button{padding:10px 20px;font-size:16px;border:none;background:#1cbcc7;color:white;border-radius:5px;cursor:pointer;}";
      html += ".dot-effect{width: 20px;aspect-ratio: 4;clip-path: inset(0 100% 0 0);animation: l1 1.3s steps(4) infinite;display: block;}@keyframes l1 {to{clip-path: inset(0 -34% 0 0)}}";
      html += "</style></head><body>";
      html += "<div class='wifi-setup-area'>";
      html += "<div class='wifi-inner'>";
      if (result != "undefined" && result == "1")
      {
        html += "<h2 class='wifi-title'>SpaPilot Wi-Fi Setup Completed</h2>";
        html += "<div class='msg'>Redirecting please wait<span class='dot-effect'></span></div>";
        restart_esp = true;
      }
      else
      {
          html += "<div class='msg'>Wrong password!!!</div>";
          html += "<button onclick=\"window.location.href='/'\">Retry</button>";
      }
      html += "</div></div>";
      html += "</body></html>";

      request->send(200, "text/html", html);
  });

//   server.on("/success", HTTP_GET, [](AsyncWebServerRequest *request) {
//     // request->send(200, "text/plain", "saved successfully!!!");
//     // request->send(200, "text/plain", "Rebooting...");

//     // delay(3000);
//     // ESP.restart();

//     // if( (WiFi.getMode() != WIFI_MODE_STA) && (WiFi.getMode() != WIFI_MODE_APSTA))
//     {
//       // WiFi.mode(WIFI_AP_STA);

//       String message = "";

//       String html = "<!DOCTYPE html><html><head><title>Wi-Fi Status</title><style>";
//       html += "body{font-family:sans-serif;background:#26221f;color:#fff;text-align:center;padding-top:50px}";
//       html += ".msg{font-size:24px;margin-bottom:20px;}";
//       html += "button{padding:10px 20px;font-size:16px;border:none;background:#1cbcc7;color:white;border-radius:5px;cursor:pointer;}";
//       html += "</style></head><body>";
      
//       if(wifiModuleCnnectToWiFiOneTime())
//       {
//         // request->send(200, "text/plain", "Rebooting...");

//         message += "Rebooting...";
//         html += "<div class='msg'>" + message + "</div>";

//         restart_esp = true;
//       }
//       else
//       {
//         // request->send(200, "text/plain", "Wrong password!!!");

//         message += "Wrong password!!!";
//         html += "<div class='msg'>" + message + "</div>";
//         html += "<button onclick=\"window.location.href='/'\">Retry</button>";
//       }

//       html += "</body></html>";

//       request->send(200, "text/html", html);
//     }
// });

  // **GET /getMacId** - Returns Wi-Fi info
  server.on("/getMacId", HTTP_GET, [](AsyncWebServerRequest *request) {
    char macAddr[18];  
    uint8_t mac[6];
    WiFi.macAddress(mac);
    sprintf(macAddr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    String json = "{ \"mac_id\": \"" + String(macAddr) + "\" }";
    request->send(200, "application/json", json);
  });

  // **POST /configUserId** - Stores user ID in Preferences
  server.on("/configUserId", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    String jsonBody = String((char*)data).substring(0, len); // Extract raw JSON
    Log.notice("[WiFi]: Received JSON: %s\n", jsonBody.c_str());

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, jsonBody);

    if (error || !doc.containsKey("user_id")) {
        request->send(400, "application/json", "{ \"error\": \"Invalid JSON or missing user_id\" }");
        return;
    }

    // Store user ID in Preferences
    String user_id = doc["user_id"].as<String>();
    preferences.putString("user_id", user_id);
    Log.notice("[WiFi]: Stored User ID: %s\n", user_id.c_str());

    request->send(200, "application/json", "{ \"message\": \"User ID stored successfully\" }");
  });

  server.begin();
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