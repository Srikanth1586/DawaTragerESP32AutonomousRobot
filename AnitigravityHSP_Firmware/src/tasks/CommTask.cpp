#include "tasks/CommTask.h"
#include "tasks/WatchdogTask.h"
#include "managers/QueueManager.h"
#include "managers/EventManager.h"
#include "managers/StateManager.h"
#include "config/robot_config.h"
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// Static instance of WebSocketsClient
static WebSocketsClient webSocket;

// Event handler for WebSocket events
static void onWebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            EventManager::getInstance().clearBits(EventManager::SERVER_CONNECTED);
            break;
            
        case WStype_CONNECTED:
            EventManager::getInstance().setBits(EventManager::SERVER_CONNECTED);
            webSocket.sendTXT("{\"status\": \"connected\", \"robot\": \"" + String(ROBOT_ID) + "\"}");
            break;
            
        case WStype_TEXT: {
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, payload, length);
            if (error) {
                return;
            }

            const char* command = doc["command"];
            if (command != nullptr) {
                NavigationCommand navCmd;
                navCmd.hasTargetSpeed = false;
                strncpy(navCmd.nextStation, "None", sizeof(navCmd.nextStation));

                if (strcmp(command, "start") == 0) {
                    navCmd.targetState = RobotState::LINE_FOLLOWING;
                    QueueManager::getInstance().sendNavigationCommand(navCmd);
                } 
                else if (strcmp(command, "stop") == 0) {
                    navCmd.targetState = RobotState::IDLE;
                    QueueManager::getInstance().sendNavigationCommand(navCmd);
                } 
                else if (strcmp(command, "estop") == 0) {
                    navCmd.targetState = RobotState::EMERGENCY_STOP;
                    QueueManager::getInstance().sendNavigationCommand(navCmd);
                } 
                else if (strcmp(command, "dock") == 0) {
                    navCmd.targetState = RobotState::CHARGING;
                    QueueManager::getInstance().sendNavigationCommand(navCmd);
                } 
                else if (strcmp(command, "set_speed") == 0) {
                    int16_t left = doc["left"] | 0;
                    int16_t right = doc["right"] | 0;
                    MotorCommand motorCmd = {left, right, false};
                    QueueManager::getInstance().sendMotorCommand(motorCmd);
                }
            }
            break;
        }
        default:
            break;
    }
}

void CommTask::run(void* pvParameters) {
    // 1. Initial WiFi configuration
    WiFi.mode(WIFI_STA);
    WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);

    // 2. Initialize WebSocket Client
    webSocket.begin(DEFAULT_WS_SERVER, DEFAULT_WS_PORT, DEFAULT_WS_PATH);
    webSocket.onEvent(onWebSocketEvent);
    webSocket.setReconnectInterval(5000); // Reconnect interval 5s

    uint32_t lastConnectionCheckMs = 0;
    CommunicationMessage txMsg;

    while (true) {
        // Run WebSocket polling loops
        webSocket.loop();

        uint32_t now = millis();

        // Check WiFi connection status every 1 second
        if (now - lastConnectionCheckMs > 1000) {
            lastConnectionCheckMs = now;
            if (WiFi.status() == WL_CONNECTED) {
                EventManager::getInstance().setBits(EventManager::WIFI_CONNECTED);
            } else {
                EventManager::getInstance().clearBits(EventManager::WIFI_CONNECTED);
                EventManager::getInstance().clearBits(EventManager::SERVER_CONNECTED);
                // Attempt reconnect
                if (WiFi.status() != WL_DISCONNECTED) {
                    WiFi.disconnect();
                }
                WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
            }
        }

        // Send outgoing messages from CommunicationQueue (non-blocking)
        if (EventManager::getInstance().getBits() & EventManager::SERVER_CONNECTED) {
            while (xQueueReceive(QueueManager::getInstance().getCommunicationQueue(), &txMsg, 0) == pdPASS) {
                webSocket.sendTXT(txMsg.payload);
            }
        }

        // Notify watchdog
        WatchdogTask::feed(COMM_TASK);

        // Short sleep to allow other tasks to execute (WebSocket client loop runs quickly)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
