#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "queue_manager.h"   
#include "structs.h"
#include "system_state.h"
#include "Config.h"
#include "Comm.h"


// ==========================
// Event Group Handle
// ==========================

extern EventGroupHandle_t robotEventGroup;
#define EVENT_OBSTACLE_DETECTED (1<<2)

// ==========================
// Init Function
// ==========================

void initEventGroups();

#endif