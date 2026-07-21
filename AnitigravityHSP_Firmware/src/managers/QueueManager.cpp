#include "managers/QueueManager.h"

QueueManager& QueueManager::getInstance() {
    static QueueManager instance;
    return instance;
}

void QueueManager::init() {
    // Instantiate all FreeRTOS queues
    m_sensorQueue        = xQueueCreate(10, sizeof(SensorData));
    m_encoderQueue       = xQueueCreate(10, sizeof(EncoderData));
    m_motorQueue         = xQueueCreate(10, sizeof(MotorCommand));
    m_nfcQueue           = xQueueCreate(5,  sizeof(NFCData));
    m_navigationQueue    = xQueueCreate(10, sizeof(NavigationCommand));
    m_displayQueue       = xQueueCreate(5,  sizeof(DisplayData));
    m_communicationQueue = xQueueCreate(20, sizeof(CommunicationMessage));
}

bool QueueManager::sendSensorData(const SensorData& data, TickType_t wait) {
    if (m_sensorQueue == nullptr) return false;
    return xQueueSend(m_sensorQueue, &data, wait) == pdPASS;
}

bool QueueManager::sendEncoderData(const EncoderData& data, TickType_t wait) {
    if (m_encoderQueue == nullptr) return false;
    return xQueueSend(m_encoderQueue, &data, wait) == pdPASS;
}

bool QueueManager::sendMotorCommand(const MotorCommand& data, TickType_t wait) {
    if (m_motorQueue == nullptr) return false;
    return xQueueSend(m_motorQueue, &data, wait) == pdPASS;
}

bool QueueManager::sendNFCData(const NFCData& data, TickType_t wait) {
    if (m_nfcQueue == nullptr) return false;
    return xQueueSend(m_nfcQueue, &data, wait) == pdPASS;
}

bool QueueManager::sendNavigationCommand(const NavigationCommand& data, TickType_t wait) {
    if (m_navigationQueue == nullptr) return false;
    return xQueueSend(m_navigationQueue, &data, wait) == pdPASS;
}

bool QueueManager::sendDisplayData(const DisplayData& data, TickType_t wait) {
    if (m_displayQueue == nullptr) return false;
    return xQueueSend(m_displayQueue, &data, wait) == pdPASS;
}

bool QueueManager::sendCommunicationMessage(const CommunicationMessage& data, TickType_t wait) {
    if (m_communicationQueue == nullptr) return false;
    return xQueueSend(m_communicationQueue, &data, wait) == pdPASS;
}

bool QueueManager::sendMotorCommandFromISR(const MotorCommand& data, BaseType_t* pxHigherPriorityTaskWoken) {
    if (m_motorQueue == nullptr) return false;
    return xQueueSendFromISR(m_motorQueue, &data, pxHigherPriorityTaskWoken) == pdPASS;
}

bool QueueManager::sendNavigationCommandFromISR(const NavigationCommand& data, BaseType_t* pxHigherPriorityTaskWoken) {
    if (m_navigationQueue == nullptr) return false;
    return xQueueSendFromISR(m_navigationQueue, &data, pxHigherPriorityTaskWoken) == pdPASS;
}
