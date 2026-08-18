#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

// ==========================
// Robot Modes
// ==========================


// ==========================
// Event Group Bits
// ==========================

#define EVENT_LINE_DETECTED      (1 << 0)
#define EVENT_NFC_DETECTED       (1 << 1)
#define EVENT_OBSTACLE           (1 << 2)
#define EVENT_STOP_REQUEST       (1 << 3)

#endif