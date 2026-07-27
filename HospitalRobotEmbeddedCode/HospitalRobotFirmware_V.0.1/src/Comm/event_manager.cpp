#include "event_manager.h"

// ==========================
// Event Group
// ==========================

EventGroupHandle_t robotEventGroup;

// ==========================
// Init Event Groups
// ==========================

void initEventGroups()
{
    robotEventGroup = xEventGroupCreate();
}