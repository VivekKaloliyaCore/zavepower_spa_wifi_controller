#include <ESPAsyncWebServer.h>
#include "httpServer.h"
#include "../wifiModule/wifiModule.h"
#include <ArduinoLog.h>
#include <ArduinoJson.h>  // Include ArduinoJson library


// Instantiate the server
AsyncWebServer server(80);

char flag_server_on = 0;

// AsyncWebServer& getServer()
// {
//     return server;
// }


void apServerOn(void)
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


void stServerOn(void)
{
  server.on("/error", HTTP_POST, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "He He");
  });

    server.on("/error", HTTP_POST, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", "He He");
  });

  server.begin();
  
  flag_server_on = 1;
}

// void serverStop(void)
// {
//     //   if(server_stops)
// //   {
// //     flag_server_on = 0;
// //   }
// }