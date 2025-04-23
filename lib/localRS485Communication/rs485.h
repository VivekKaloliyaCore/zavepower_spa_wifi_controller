#ifndef RS485_H
#define RS485_H

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include "../../src/main.h"

#define RS485_WRITE_QUEUE 10

void rs485Setup();
void rs485Loop();
void rs485ClearToSend();

// Test:::
void switchTempRange(void); // Tested and working; Switches temp range from low to high and high to low;
void switchHeatMode(void); // Tesd and working; Switch heat mode from Ready to Rest and Rest to Ready;
void setTemp(int temp); // Tested; Pass value of temperature needed to be set as an argument in Hex & according to current temperature range;
void getTemp(void);

// void rs485Send(uint8_t *data, int length, boolean addCrc, boolean force = false);
// void rs485Send(CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> &data, boolean addCrc, boolean force = false);
void addCRC(CircularBuffer<uint8_t, BALBOA_MESSAGE_SIZE> &data);

extern uint8_t id; // spa id

struct Rs485Stats
{
  uint32_t messagesToday;
  uint32_t messagesYesterday;
  uint32_t badFormatToday;
  uint32_t badFormatYesterday;
  uint32_t crcToday;
  uint32_t crcYesterday;
  uint32_t magicNumber;
};

extern Rs485Stats rs485Stats;

struct rs485WriteQueueMessage
{
  char message[BALBOA_MESSAGE_SIZE];
  int length;
};

#endif