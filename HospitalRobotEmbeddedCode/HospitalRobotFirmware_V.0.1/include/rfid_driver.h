#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include <Arduino.h>
#include "rfid_data.h"

bool rfidInit();
bool readRFID(char *uidBuffer, size_t bufferSize);

// Lookup RFID card information by UID
void getRoomInfoFromUID(const char *uid, RFIDResult_t *result);

#endif
