#ifndef MANAGERS_EVENT_MANAGER_H
#define MANAGERS_EVENT_MANAGER_H

#include <Arduino.h>

// FreeRTOS libraries
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

class EventManager {
public:
    // Event group bit mappings
    static constexpr EventBits_t WIFI_CONNECTED      = (1 << 0);
    static constexpr EventBits_t SERVER_CONNECTED    = (1 << 1);
    static constexpr EventBits_t LINE_DETECTED        = (1 << 2);
    static constexpr EventBits_t OBSTACLE_PRESENT     = (1 << 3);
    static constexpr EventBits_t NFC_DETECTED         = (1 << 4);
    static constexpr EventBits_t CHARGING_ACTIVE      = (1 << 5);
    static constexpr EventBits_t EMERGENCY_STOP       = (1 << 6);
    static constexpr EventBits_t LOW_BATTERY          = (1 << 7);

    // Singleton accessor
    static EventManager& getInstance();

    // Prevent copying
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // Core methods
    void init();
    EventBits_t setBits(EventBits_t bitsToSet);
    EventBits_t clearBits(EventBits_t bitsToClear);
    EventBits_t getBits() const;
    EventBits_t waitBits(EventBits_t bitsToWaitFor, bool clearOnExit, bool waitForAllBits, TickType_t ticksToWait);

    // ISR-safe methods
    BaseType_t setBitsFromISR(EventBits_t bitsToSet, BaseType_t* pxHigherPriorityTaskWoken);
    BaseType_t clearBitsFromISR(EventBits_t bitsToClear);

private:
    EventManager() = default;
    ~EventManager() = default;

    EventGroupHandle_t m_eventGroup = nullptr;
};

#endif // MANAGERS_EVENT_MANAGER_H
