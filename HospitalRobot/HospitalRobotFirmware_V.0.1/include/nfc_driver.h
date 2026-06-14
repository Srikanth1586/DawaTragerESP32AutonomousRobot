#ifndef NFC_DRIVER_H
#define NFC_DRIVER_H

#include <Arduino.h>

bool nfcInit();
bool readNFC(char *uidBuffer);

#endif