#include "managers/EventManager.h"

EventManager& EventManager::getInstance() {
    static EventManager instance;
    return instance;
}

void EventManager::init() {
    if (m_eventGroup == nullptr) {
        m_eventGroup = xEventGroupCreate();
    }
}

EventBits_t EventManager::setBits(EventBits_t bitsToSet) {
    if (m_eventGroup == nullptr) return 0;
    return xEventGroupSetBits(m_eventGroup, bitsToSet);
}

EventBits_t EventManager::clearBits(EventBits_t bitsToClear) {
    if (m_eventGroup == nullptr) return 0;
    return xEventGroupClearBits(m_eventGroup, bitsToClear);
}

EventBits_t EventManager::getBits() const {
    if (m_eventGroup == nullptr) return 0;
    return xEventGroupGetBits(m_eventGroup);
}

EventBits_t EventManager::waitBits(EventBits_t bitsToWaitFor, bool clearOnExit, bool waitForAllBits, TickType_t ticksToWait) {
    if (m_eventGroup == nullptr) return 0;
    return xEventGroupWaitBits(
        m_eventGroup,
        bitsToWaitFor,
        clearOnExit ? pdTRUE : pdFALSE,
        waitForAllBits ? pdTRUE : pdFALSE,
        ticksToWait
    );
}

BaseType_t EventManager::setBitsFromISR(EventBits_t bitsToSet, BaseType_t* pxHigherPriorityTaskWoken) {
    if (m_eventGroup == nullptr) return pdFAIL;
    return xEventGroupSetBitsFromISR(m_eventGroup, bitsToSet, pxHigherPriorityTaskWoken);
}

BaseType_t EventManager::clearBitsFromISR(EventBits_t bitsToClear) {
    if (m_eventGroup == nullptr) return pdFAIL;
    return xEventGroupClearBitsFromISR(m_eventGroup, bitsToClear);
}
