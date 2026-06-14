#include "Comm.h"
#include "Config.h"
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "structs.h"
#include "queue_manager.h"
#include "event_manager.h"
#include "system_state.h"
#include "robot_state.h"
#include "servo_driver.h" 
// ======================================================
// WEBSOCKET OBJECT
// ======================================================

WebSocketsClient webSocket;

// Server connection status
bool serverConnected = false;
extern RobotState robotState;
// ======================================================
// JSON DATA STRUCTURE
// ======================================================
static bool extractRobotCommand(
    const String& json,
    RobotCommand_t& cmd)
{
    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, json);

    if(error)
    {
        Serial.println("[WS] JSON Parse Failed");
        return false;
    }
    Serial.println("[WS] JSON Parsed Successfully");
    cmd.type =
        doc["type"] | "";

    cmd.robotId =
        doc["robotId"] | "";

    cmd.command =
        doc["command"] | "";

    cmd.roomNumber =
        doc["task"]["roomNumber"] | 0;

    return true;
}



// ======================================================
// WIFI CONFIGURATION
// ======================================================

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ======================================================
// WEBSOCKET SERVER
// ======================================================

// Replace with your PC IP

const char* websocket_host = WEBSOCKET_HOST;
const uint16_t websocket_port = WEBSOCKET_PORT;

const char* websocket_path = WEBSOCKET_PATH;

// ======================================================
// ROBOT CONFIG
// ======================================================

String robotID = ROBOT_ID;



// ======================================================
// FREERTOS QUEUE
// ======================================================

QueueHandle_t messageQueue;
//=====================================================
// JSON DATA ExTRACTION STRUCTURE
// ======================================================
extern QueueHandle_t commandQueue;

// ======================================================
// WIFI INITIALIZATION
// ======================================================

bool initWiFi()
{
    Serial.println();
    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, password);

    int attempts = 0;
    while(WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        Serial.print(".");
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;
    }

    Serial.println();
    
    if(WiFi.status() == WL_CONNECTED)
    {
        Serial.println("WiFi Connected");
        Serial.print("ESP32 IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("WiFi Connection Failed");
        return false;
    }
}

// ======================================================
// GET WIFI STATUS
// ======================================================

bool getWiFiStatus()
{
    return (WiFi.status() == WL_CONNECTED);
}

// ======================================================
// HEARTBEAT JSON
// ======================================================

void sendHeartbeat()
{
    JsonDocument doc;

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
    JsonDocument doc;

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
// GET SERVER CONNECTION STATUS
// ======================================================

bool getServerStatus()
{
    return serverConnected;
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
        case WStype_CONNECTED:
        {
            Serial.println("[WS] Connected to Server" );
            serverConnected = true;
            sendRobotMessage("ESP32 Connected");

            break;
        }

        // ==========================================
        // DISCONNECTED
        // ==========================================

        case WStype_DISCONNECTED:
        {
            Serial.println("[WS] Disconnected");
            serverConnected = false;
            break;
        }

        // ==========================================
        // TEXT MESSAGE RECEIVED
        // ==========================================

             case WStype_TEXT:
        {
        String receivedMessage =
            String((char*)payload);

        RobotCommand_t cmd;

            if(extractRobotCommand( receivedMessage, cmd))
            {
            Serial.println("----- COMMAND -----");
            Serial.println(receivedMessage);
            Serial.println(cmd.type);
            Serial.println(cmd.robotId);
            Serial.println(cmd.command);
            Serial.println(cmd.roomNumber);

            // ==========================================
            // HANDLE DOOR COMMANDS IMMEDIATELY
            // ==========================================
            
            if(cmd.command == "open_l_box_door")
            {
                controlDoor(DOOR_L_BOX, DOOR_OPEN);
                sendRobotMessage("L_BOX_DOOR opened");
            }
            else if(cmd.command == "close_l_box_door")
            {
                controlDoor(DOOR_L_BOX, DOOR_CLOSE);
                sendRobotMessage("L_BOX_DOOR closed");
            }
            else if(cmd.command == "open_m_box_door")
            {
                controlDoor(DOOR_M_BOX, DOOR_OPEN);
                sendRobotMessage("M_BOX_DOOR opened");
            }
            else if(cmd.command == "close_m_box_door")
            {
                controlDoor(DOOR_M_BOX, DOOR_CLOSE);
                sendRobotMessage("M_BOX_DOOR closed");
            }
            else
            {
                // Queue command for other processing
                xQueueSend(commandQueue, &cmd, 0);
            }
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
        // UPDATE SERVER STATUS IN ROBOT STATE
        // ==========================================
        robotState.serverConnected = serverConnected;

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