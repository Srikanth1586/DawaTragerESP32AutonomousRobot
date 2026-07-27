#ifndef TOF_DRIVER_H
#define TOF_DRIVER_H

#include <Arduino.h>

bool tofInit();
uint16_t readDistance();

#endif