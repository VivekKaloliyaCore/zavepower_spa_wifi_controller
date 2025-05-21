#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "../spaMessage/spaMessage.h"

void httpStart(void)
{
  if(flagsendErrorCode == 1)
  {
    flagsendErrorCode = 0;
    DynamicJsonDocument doc(200);

    // Add key-value pairs
    doc["action"] = "response";
    doc["msgT"] = "errorCodes";

    // Create "payload" as a nested object
    JsonObject payload = doc.createNestedObject("payload");
    
    payload["macAddress"] = "ready";
    if(flagHeaterTooHigh == 1)
    {
      flagHeaterTooHigh = 0;
      payload["ErrorCode"] = "30 - Heater Too High";
      payload["Description"] = "The spa-water's temerature is too high";
    }
    else if(flagSetTime == 1)
    {
      flagSetTime = 0;
      payload["ErrorCode"] = "30 - Heater Too High";
      payload["Description"] = "The spa-water's temerature is too high";
    }
    

    



    HTTPClient cli;

    cli.begin("https://admin.corefragment.com/api/errorCodes");

    int httpResponseCode = cli.POST("");

    if (httpResponseCode > 0) 
    {
      String response = cli.getString();  // Store response
      Serial.println("Response code: " + String(httpResponseCode)); 
      Serial.println("Response body: " + response);
    } 
    else 
    {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    cli.end();
  }
}