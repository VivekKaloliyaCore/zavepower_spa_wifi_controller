#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <TickTwo.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Update.h>
#include "HttpsOTAUpdate.h"
#include <HTTPClient.h>
#include <Update.h>

// Local Libraries
#include <wifiModule.h>
#include <spaUtilities.h>
#include <restartReason.h>
#include "mqttModule.h"
#include <rs485.h>
#include "spaMessage.h"
#include "spaControl.h"
#include "spaMqttMessage.h"
#include "httpsClient.h"
// Local Functions
void reconnect();
void mqttMessage(char *p_topic, byte *p_payload, unsigned int p_length);
void nodeStateReport();

// WiFiClient wifiClient;
// PubSubClient mqtt(wifiClient);
WiFiClientSecure wifiClientSecure;
PubSubClient mqtt(wifiClientSecure);
String mqttTopic = "Spa/"; // root topic, gets appeanded with node mac address

// TickTwo sendStatus(nodeStateReport, 1.5 * 60 * 1000); // 5 minutes

/* OTA */
static bool otaUpdateRunning = false;
static HttpsOTAStatus_t otastatus;
void HttpEvent(HttpEvent_t *event)
{
  switch (event->event_id) {
    case HTTP_EVENT_ERROR:        Log.notice("Http Event Error\n"); break;
    case HTTP_EVENT_ON_CONNECTED: Log.notice("Http Event On Connected\n"); break;
    case HTTP_EVENT_HEADER_SENT:  Log.notice("Http Event Header Sent\n"); break;
    case HTTP_EVENT_ON_HEADER:    Log.notice("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value); break;
    case HTTP_EVENT_ON_DATA:      break;
    case HTTP_EVENT_ON_FINISH:    Log.notice("Http Event On Finish\n"); break;
    case HTTP_EVENT_DISCONNECTED: Log.notice("Http Event Disconnected\n"); break;
    // case HTTP_EVENT_REDIRECT:     Log.notice("Http Event Redirect\n"); break;
  }
}

void mqttModuleSetup()
{
  wifiClientSecure.setCACert(AWS_CERT_CA);
  wifiClientSecure.setCertificate(AWS_CERT_CRT);
  wifiClientSecure.setPrivateKey(AWS_CERT_PRIVATE);
  // mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setServer(MQTTS_SERVER, MQTTS_PORT);
  mqtt.setCallback(mqttMessage);
  mqtt.setKeepAlive(10);
  mqtt.setSocketTimeout(20);
  mqttTopic = mqttTopic + String(gatewayName) + "/";
  // Log.notice("MQTT Server: %s:%d\n", MQTT_SERVER, MQTT_PORT);
  Log.notice("MQTT Server: %s:%d\n", MQTTS_SERVER, MQTTS_PORT);
  Log.notice("MQTT Topic: %s\n", mqttTopic.c_str());
  // sendStatus.start();

  // sprintf(&mqtt_params.mqtt_topic_postfix[0], "response");
  
  // mqtt_params_t mqtt_params = {0};
  // memset(&mqtt_params, 0, sizeof(mqtt_params_t));
  // mqtt_params.is_mqtt_topic_postfix_present = true;
  // sprintf(&mqtt_params.mqtt_topic_postfix[0], "response");
  // set_mqtt_params(mqtt_params);
  // spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], NULL, 0);
  // spaMqttMessage_publish_message("response/server", NULL, 0);
  // spaMqttMessage_publish_message("debug", NULL, 0);
  // spaMqttMessage_publish_message("bridge/msg", NULL, 0);
  // spaMqttMessage_publish_message("debug/message", NULL, 0);
  // spaMqttMessage_publish_message("debug/error", NULL, 0);
  // spaMqttMessage_publish_message("status", NULL, 0);
}

void mqttModuleLoop()
{
  // TODO: Implement MQTT module loop
  if (!mqtt.connected())
  {
    reconnect();
  }
  // sendStatus.update();
  mqtt.loop();
  spaControl_mqtt_action();


  if(otaUpdateRunning)
  {
    otastatus = HttpsOTA.status();
    if (otastatus == HTTPS_OTA_SUCCESS)
    {
      otaUpdateRunning = false;
      Log.notice("OTA completed. Firmware written successfully.\n");
      Log.notice("Restarting in 5 sec...\n");
      delay(5000);
      ESP.restart();
    }
    else if (otastatus == HTTPS_OTA_FAIL)
    {
      otaUpdateRunning = false;
      Log.notice("OTA failed. Firmware Upgrade Fail!!!\n");
    }
  }
}

void sendErrorCode(char *errorMessage[])
{
char post_data[64];
    snprintf(post_data, sizeof(post_data), "{\"temperature\": %s}", errorMessage);

    esp_http_client_config_t config = {
        .url = "http://example.com/temperature",  // 🔁 Replace with your real endpoint
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    // esp_err_t err = esp_http_client_perform(client);
    esp_http_client_perform(client);

    // if (err == ESP_OK) {
    //     ESP_LOGI(TAG, "Status = %d, content_length = %d",
    //              esp_http_client_get_status_code(client),
    //              esp_http_client_get_content_length(client));
    // } else {
    //     ESP_LOGE(TAG, "POST failed: %s", esp_err_to_name(err));
    // }

    esp_http_client_cleanup(client);
}

void reconnect()
{
  // int oldstate = mqtt.state();
  // boolean connection = false;
  //  Loop until we're reconnected
  if (!mqtt.connected())
  {
    // Attempt to connect

    // connection =
    // mqtt.connect(gatewayName, BROKER_LOGIN, BROKER_PASS, (mqttTopic + "node/state").c_str(), 1, true, "OFF");
    mqtt.connect(gatewayName, (mqttTopic + "node/state").c_str(), 1, true, "OFF");

    // time to connect
    delay(1000);

    if (mqtt.connected())
    {
      mqtt_params_t mqtt_params = {0};
      memset(&mqtt_params, 0, sizeof(mqtt_params_t));
      mqtt_params.is_mqtt_topic_postfix_present = true;
      sprintf(&mqtt_params.mqtt_topic_postfix[0], "response");
      set_mqtt_params(mqtt_params);

      // spaMqttMessage_publish_message(&mqtt_params.mqtt_topic_postfix[0], NULL, 0);
      // spaMqttMessage_publish_message("response/server", NULL, 0);
      // spaMqttMessage_publish_message("debug", NULL, 0);
      // spaMqttMessage_publish_message("bridge/msg", NULL, 0);
      // spaMqttMessage_publish_message("debug/message", NULL, 0);
      // spaMqttMessage_publish_message("debug/error", NULL, 0);
      // spaMqttMessage_publish_message("status", NULL, 0);

      publishError("MQTT Timeout - Reconnect Successfully Run", false);
      mqtt.subscribe((mqttTopic + "command").c_str());
      mqtt.subscribe((mqttTopic + "command/server").c_str());
      
      spaControlStatus_t spaControlStatus = {0};
      memset(&spaControlStatus, 0, sizeof(spaControlStatus_t));
      spaControlStatus.bootupPacket = true;
      set_spaControlStatus(spaControlStatus);


      httpStart();
      delay(2000);
      Log.notice("http Started \n");

      // nodeStateReport();
    }
  }
  mqtt.setBufferSize(512); // increase pubsubclient buffer size
}

void mqttMessage(char *p_topic, byte *p_payload, unsigned int p_length)
{
  // Log.notice(">>>> %s | %d | %s\n", p_topic, p_length, p_payload);

  mqtt_params_t mqtt_params = {0};
  memset(&mqtt_params, 0, sizeof(mqtt_params_t));
  if(strstr(p_topic, "/server"))
  {
    mqtt_params.is_mqtt_topic_postfix_present = true;
    sprintf(&mqtt_params.mqtt_topic_postfix[0], "response/server");
    set_mqtt_params(mqtt_params);
  }
  else
  {
    mqtt_params.is_mqtt_topic_postfix_present = true;
    sprintf(&mqtt_params.mqtt_topic_postfix[0], "response");
    set_mqtt_params(mqtt_params);
  }

  memset(&mqtt_params, 0, sizeof(mqtt_params_t));
  mqtt_params.is_parse_mqtt_msg_present = true;
  mqtt_params.parse_mqtt_msg = true;
  mqtt_params.mqtt_msg_len = p_length;
  memset(mqtt_params.mqtt_msg, 0, sizeof(mqtt_params.mqtt_msg));
  memcpy(mqtt_params.mqtt_msg, (char *)p_payload, p_length);
  set_mqtt_params(mqtt_params);

  // char payload[1024] = {0};
  // memset(payload, 0, sizeof(payload));
  // memcpy(payload, (char *)p_payload, p_length);

  // spaControlParams_t spaControlParams = {0};
  // memset(&spaControlParams, 0, sizeof(spaControlParams_t));

  // spaControlStatus_t spaControlStatus = {0};
  // memset(&spaControlStatus, 0, sizeof(spaControlStatus_t));

  // otaParams_t otaParams = {0};
  // memset(&otaParams, 0, sizeof(otaParams_t));

  // if(spaControl_parse_action_command((char *)payload, &spaControlParams, &spaControlStatus, &otaParams))
  // {

  //   Log.notice("parse_action_command\n");
  //   if(spaControlStatus.deviceStatus)
  //   {
  //     Log.notice("Sending deviceStatus...\n");
  //     set_spaControlStatus(spaControlStatus);
  //   }
  //   else if(spaControlStatus.currentTemp)
  //   {
  //     Log.notice("Sending currentTemp...\n");
  //     set_spaControlStatus(spaControlStatus);
  //   }
  //   else if(spaControlStatus.setTemp)
  //   {
  //     Log.notice("Sending setTemp...\n");
  //     set_spaControlStatus(spaControlStatus);
  //   }
  //   else if(spaControlStatus.heatMode)
  //   {
  //     Log.notice("Sending heatMode...\n");
  //     set_spaControlStatus(spaControlStatus);
  //   }
  //   else if(spaControlStatus.tempRange)
  //   {
  //     Log.notice("Sending tempRange...\n");
  //     set_spaControlStatus(spaControlStatus);
  //   }
  //   else if(spaControlStatus.device_info)
  //   {
  //     set_spaControlStatus(spaControlStatus);
  //   }
  //   else if(spaControlStatus.filterCycle)
  //   {
  //     set_spaControlStatus(spaControlStatus);
  //   }

  //   if(spaControlParams.is_jet1_present)
  //   {
  //     Log.notice("Sending jet 1 command...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_jet2_present)
  //   {
  //     Log.notice("Sending jet 2 command...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_jet3_present)
  //   {
  //     Log.notice("Sending jet 3 command...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_jet4_present)
  //   {
  //     Log.notice("Sending jet 4 command...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_blower1_present)
  //   {
  //     Log.notice("Sending Blower 1 command...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_light1_present)
  //   {
  //     Log.notice("Sending Light 1 command...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_reset_wifi_sta_present)
  //   {
  //     Log.notice("Reset WiFi STA...\n");

  //     if(spaControlParams.reset_wifi_sta)
  //     {
  //       wifiModuleEraseStaConfig();

  //       Log.notice("WiFi STA config reset. Rebooting...\n");
  //       delay(2000);
  //       ESP.restart();
  //     }

  //     // set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_temp_range_high_present)
  //   {
  //     // Log.notice("Sending currentTemp...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_temp_range_low_present)
  //   {
  //     // Log.notice("Sending currentTemp...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_ready_mode_present)
  //   {
  //     // Log.notice("Sending currentTemp...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_resting_mode_present)
  //   {
  //     // Log.notice("Sending currentTemp...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_set_temp_present)
  //   {
  //     // Log.notice("Sending currentTemp...\n");
  //     set_spaControlParams(spaControlParams);
  //   }
  //   else if(spaControlParams.is_filterCycle_present)
  //   {
  //     // Log.notice("Sending currentTemp...\n");
  //     set_spaControlParams(spaControlParams);
  //   }

  //   if(otaParams.is_url_present)
  //   {
  //     String message;
  //     for (unsigned int i = 0; i < strlen(otaParams.url); i++) {
  //         message += (char)otaParams.url[i];
  //     }
  
  //     Log.notice("Firmware URL: %s\n", message.c_str());
      
  //     if(strstr(message.c_str(), "https"))
  //     {
  //       performOTA(message);
  //     }
  //     else
  //     {
  //       performOTA_unsecured(message);
  //     }
  //   }
  // }

  // if(strstr((char *)p_payload, "jet 1"))
  // {
  //   Log.notice("Sending JET command...\n");

  //   spaControlParams_t spaControlParams = get_spaControlParams();
  //   spaControlParams.is_jet1_present = true;
  //   spaControlParams.jet1 = 1;
  //   set_spaControlParams(spaControlParams);
  // }

  // mqtt.publish(p_topic, p_payload, p_length);
}

void nodeStateReport()
{
  if (mqtt.connected())
  {
    publishNodeStatus("ip", WiFi.localIP().toString().c_str(), false);
    publishNodeStatus("mac", WiFi.macAddress().c_str(), false);
    publishNodeStatus("gateway", gatewayName, false);
    publishNodeStatus("restartReason", getLastRestartReason().c_str(), false);
    publishNodeStatus("uptime", String(millis() / 1000).c_str(), false);
    publishNodeStatus("getTime", String(getTime()).c_str(), false);
    publishNodeStatus("state", "ON", false);
    publishNodeStatus("flashsize", String(ESP.getFlashChipSize()).c_str(), false);
    publishNodeStatus("chipid", String(ESP.getChipModel()).c_str(), false);
    publishNodeStatus("speed", String(ESP.getCpuFreqMHz()).c_str(), false);
    publishNodeStatus("heap", String(ESP.getFreeHeap()).c_str(), false);
    publishNodeStatus("psram", String(ESP.getFreePsram()).c_str(), false);
    publishNodeStatus("stack", String(uxTaskGetStackHighWaterMark(NULL)).c_str(), false);

    publishNodeStatus("rs485 messagesToday", String(rs485Stats.messagesToday).c_str(), false);
    publishNodeStatus("rs485 crcToday", String(rs485Stats.crcToday).c_str(), false);
    publishNodeStatus("rs485 messagesYesterday", String(rs485Stats.messagesYesterday).c_str(), false);
    publishNodeStatus("rs485 crcYesterday", String(rs485Stats.crcYesterday).c_str(), false);
    publishNodeStatus("rs485 badFormatToday", String(rs485Stats.badFormatToday).c_str(), false);
    publishNodeStatus("rs485 badFormatYesterday", String(rs485Stats.badFormatYesterday).c_str(), false);

    String release = String(__DATE__) + " - " + String(__TIME__);
    publishNodeStatus("release", release.c_str(), false);
    publishNodeStatus("buildDefinition", buildDefinitionString.c_str(), false);
  }
}

void performOTA(String firmwareURL)
{
  HttpsOTA.onHttpEvent(HttpEvent);
  char fw_url[512];
  memset(&fw_url[0], 0, sizeof(fw_url));
  memcpy(&fw_url[0], firmwareURL.c_str(), strlen(firmwareURL.c_str()));
  Log.notice("Starting OTA from URL: %s\n", fw_url);
  HttpsOTA.begin(fw_url, ota_server_certificate);

  otaUpdateRunning = true;
}

void performOTA_unsecured(String firmwareURL)
{
    Log.notice("Starting OTA from URL: %s\n", firmwareURL.c_str());

    HTTPClient http;
    http.begin(firmwareURL);  // Start HTTP connection

    int httpCode = http.GET();  // Send HTTP GET request

    if (httpCode == HTTP_CODE_OK) {  // Check for successful response
        int contentLength = http.getSize();
        bool canBegin = Update.begin(contentLength);

        if (canBegin) {
            WiFiClient* stream = http.getStreamPtr();
            Log.notice("Flashing firmware...\n");

            size_t written = Update.writeStream(*stream);

            if (written == contentLength) {
              Log.notice("OTA completed. Firmware written successfully.\n");
              // Log.notice("Restarting in 5 sec...\n");
              // delay(5000);
              // ESP.restart();
            } else {
              Log.notice("Firmware write failed! Only %d bytes written out of %d\n", written, contentLength);
            }

            if (Update.end()) {
              Log.notice("Update finished!\n");
                if (Update.isFinished()) {
                    Log.notice("Restarting in 5 sec...\n");
                    delay(5000);
                    ESP.restart();
                } else {
                    Log.notice("Update not finished? Something went wrong.\n");
                }
            } else {
                Log.notice("Update failed! Error: %s\n", Update.errorString());
            }
        } else {
            Log.notice("Not enough space for OTA update!\n");
        }
    } else {
        Log.notice("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}