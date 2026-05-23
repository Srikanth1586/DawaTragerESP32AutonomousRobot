#include "Comm.h"

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// ======================================================
// WIFI CONFIGURATION
// ======================================================

const char* ssid = "Bommarillu 2.4G";
const char* password = "theboys@123";

// ======================================================
// WEBSOCKET SERVER
// ======================================================

// Replace with your PC IP

const char* websocket_host = "192.168.178.66";

const uint16_t websocket_port = 3000;

const char* websocket_path = "/ws/robot-chat";

// ======================================================
// ROBOT CONFIG
// ======================================================

String robotID = "RBT-001";

// ======================================================
// WEBSOCKET OBJECT
// ======================================================

WebSocketsClient webSocket;

// ======================================================
// FREERTOS QUEUE
// ======================================================

QueueHandle_t messageQueue;

// ======================================================
// WIFI INITIALIZATION
// ======================================================

void initWiFi()
{
    Serial.println();
    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    Serial.println();
    Serial.println("WiFi Connected");

    Serial.print("ESP32 IP: ");

    Serial.println(WiFi.localIP());
}

// ======================================================
// HEARTBEAT JSON
// ======================================================

void sendHeartbeat()
{
    StaticJsonDocument<200> doc;

    doc["type"] = "heartbeat";

    doc["robotId"] = robotID;

    String jsonString;

    serializeJson(doc, jsonString);

    webSocket.sendTXT(jsonString);

    Serial.println("Heartbeat Sent");
}

// ======================================================
// ROBOT MESSAGE JSON
// ======================================================

void sendRobotMessage(String message)
{
    StaticJsonDocument<256> doc;

    doc["type"] = "robot_message";

    doc["robotId"] = robotID;

    doc["message"] = message;

    String jsonString;

    serializeJson(doc, jsonString);

    webSocket.sendTXT(jsonString);

    Serial.print("Message Sent: ");

    Serial.println(jsonString);
}

// ======================================================
// WEBSOCKET EVENT CALLBACK
// ======================================================

void webSocketEvent(WStype_t type,
                    uint8_t * payload,
                    size_t length)
{
    switch(type)
    {
        // ==========================================
        // CONNECTED
        // ==========================================
        Serial.println(type);

        case WStype_CONNECTED:
        {
            Serial.println("[WS] Connected to Server" );

            sendRobotMessage("ESP32 Connected");

            break;
        }

        // ==========================================
        // DISCONNECTED
        // ==========================================

        case WStype_DISCONNECTED:
        {
            Serial.println("[WS] Disconnected");

            break;
        }

        // ==========================================
        // TEXT MESSAGE RECEIVED
        // ==========================================

        case WStype_TEXT:
        {
            String receivedMessage =
            String((char*)payload);

            Serial.print("[WS] Received: ");

            Serial.println(receivedMessage);

            // ======================================
            // CHECK SERVER COMMANDS
            // ======================================

            if(receivedMessage == "Are you there?")
            {
                // Send response immediately

                sendRobotMessage("I am alive");
            }

            // Example Future Command

            if(receivedMessage == "GO_TO_PHARMACY")
            {
                Serial.println("Navigation Command Received");

                String msg = "Moving to Pharmacy";

                xQueueSend(messageQueue,
                           &msg,
                           portMAX_DELAY);
            }

            break;
        }

        default:
            break;
    }
}

// ======================================================
// WEBSOCKET INITIALIZATION
// ======================================================

void initWebSocket()
{
    webSocket.begin(
        websocket_host,
        websocket_port,
        websocket_path
    );

    webSocket.onEvent(webSocketEvent);

    webSocket.setReconnectInterval(5000);

    Serial.println("WebSocket Initialized");
}

// ======================================================
// COMMUNICATION TASK
// ======================================================

void communicationTask(void *pvParameters)
{
    TickType_t lastHeartbeat = 0;

    String outgoingMessage;

    while(true)
    {
        // ==========================================
        // KEEP WEBSOCKET ALIVE
        // ==========================================

        webSocket.loop();

        // ==========================================
        // SEND HEARTBEAT EVERY 5 SEC
        // ==========================================

        if(xTaskGetTickCount() - lastHeartbeat >
           pdMS_TO_TICKS(5000))
        {
            sendHeartbeat();

            lastHeartbeat = xTaskGetTickCount();
        }

        // ==========================================
        // SEND QUEUED MESSAGES
        // ==========================================

        if(xQueueReceive(messageQueue,
                         &outgoingMessage,
                         200))
        {
            sendRobotMessage(outgoingMessage);
        }

        // ==========================================
        // TASK DELAY
        // ==========================================

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}