#include <HTTPClient.h>

void httpStart(void)
{
    HTTPClient cli;

    cli.begin("https://admin.corefragment.com/api/errorCodes");

    int httpResponseCode = cli.POST("");

    if (httpResponseCode > 0) {
      String response = cli.getString();  // Store response
      Serial.println("Response code: " + String(httpResponseCode)); 
      Serial.println("Response body: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }
    cli.end();
}