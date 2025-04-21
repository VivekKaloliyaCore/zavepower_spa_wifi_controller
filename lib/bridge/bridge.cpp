#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoLog.h>

#include  <spaUtilities.h>
#include <mqttModule.h>
#include <rs485.h>

#include "bridge.h"
#include "spaMessage.h"
#include "balboa.h"

#define publishBridge(...) mqtt.publish((mqttTopic + "bridge/msg").c_str(), __VA_ARGS__)

// Local functions

CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> Q_out;

uint8_t message[BALBOA_MESSAGE_SIZE];
WiFiUDP Discovery;

char packetBuffer[255]; // buffer to hold incoming packet
char replyBuffer[30];

bool bridgeStarted = false;