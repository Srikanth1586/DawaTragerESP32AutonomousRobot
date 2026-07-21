#include <Arduino.h>

#include "queue_manager.h"
#include "event_manager.h"
#include "structs.h"
#include "system_state.h"
#include "Config.h"
#include "Comm.h"
#include "robot_state.h"
#include "servo_driver.h"

RobotMode currentState = STATE_SEARCH_DOCK;

MotorCommand_t motorCmd;
EncoderData_t encData;
ToFData_t tofData;
char destinationRoom[16] = "";
bool adminDoorOpen = false;

// ==========================
// CONTROL TASK
// ==========================

void ControlTask(void *pvParameters)
{
    EventBits_t events;
    RobotCommand_t cmd;
    NFCData_t nfcData;

    Serial.println("Control Task Started");

    while (1)
    {
        // ==========================
        // 1. Check Events
        // ==========================
        events = xEventGroupGetBits(robotEventGroup);

        // ==========================
        // 2. PRIORITY: STOP EVENT
        // ==========================
        if (events & EVENT_STOP_REQUEST)
        {
            currentState = STATE_STOP;
            strcpy(robotState.currentTask, "STOP");
            strncpy(robotState.statusMessage, "Emergency stop", sizeof(robotState.statusMessage) - 1);
            robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

            motorCmd.motion = MOTION_STOP;
            motorCmd.leftSpeed = 0;
            motorCmd.rightSpeed = 0;
            xQueueSend(motorQueue, &motorCmd, 0);
        }

        // ==========================
        // 3. COMMAND RECEIVED
        // ==========================
        if (xQueueReceive(commandQueue, &cmd, pdMS_TO_TICKS(10)) == pdPASS)
        {
            Serial.println("CONTROL TASK");
            Serial.print("Command : ");
            Serial.println(cmd.command);

            if (cmd.command == "go_to_pharmacy")
            {
                currentState = STATE_GO_TO_PHARMACY;
                strcpy(robotState.currentTask, "GO PHARMACY");
                strncpy(robotState.statusMessage, "Heading to pharmacy", sizeof(robotState.statusMessage) - 1);
                robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';
            }
            else if (cmd.command == "stop_at_pharmacy")
            {
                currentState = STATE_AT_PHARMACY_WAIT_DOOR;
                strcpy(robotState.currentTask, "AT PHARMACY");
                strncpy(robotState.statusMessage, "Waiting for pharmacist", sizeof(robotState.statusMessage) - 1);
                robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

                motorCmd.motion = MOTION_STOP;
                motorCmd.leftSpeed = 0;
                motorCmd.rightSpeed = 0;
                xQueueSend(motorQueue, &motorCmd, 0);
            }
            else if (cmd.command == "deliver_to_destination")
            {
                currentState = STATE_SEARCH_DESTINATION;
                strcpy(robotState.currentTask, "DELIVERING");
                strncpy(robotState.statusMessage, "Searching destination", sizeof(robotState.statusMessage) - 1);
                robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';
                strncpy(destinationRoom, cmd.roomNumber, sizeof(destinationRoom) - 1);
                destinationRoom[sizeof(destinationRoom) - 1] = '\0';
            }
            else if (cmd.command == "wait_for_nurse")
            {
                currentState = STATE_UNLOADING_MEDICINE;
                strcpy(robotState.currentTask, "UNLOADING");
                strncpy(robotState.statusMessage, "Awaiting admin RFID", sizeof(robotState.statusMessage) - 1);
                robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

                motorCmd.motion = MOTION_STOP;
                motorCmd.leftSpeed = 0;
                motorCmd.rightSpeed = 0;
                xQueueSend(motorQueue, &motorCmd, 0);
            }
            else if (cmd.command == "idle")
            {
                currentState = STATE_WAIT_ORDER;
                strcpy(robotState.currentTask, "IDLE");
                strncpy(robotState.statusMessage, "Waiting at dock", sizeof(robotState.statusMessage) - 1);
                robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

                motorCmd.motion = MOTION_STOP;
                motorCmd.leftSpeed = 0;
                motorCmd.rightSpeed = 0;
                xQueueSend(motorQueue, &motorCmd, 0);
            }
            else
            {
                Serial.println("Unknown command received");
            }
        }

        // ==========================
        // 4. NFC EVENT HANDLING
        // ==========================
        if (events & EVENT_NFC_DETECTED)
        {
            if (xQueueReceive(nfcQueue, &nfcData, 0) == pdPASS)
            {
                RFIDResult_t card = getRFIDInfo(nfcData.uid);

                Serial.println("═══════════════════════════════════");
                Serial.println("📍 RFID CARD DETECTED!");
                Serial.print("Card UID: ");
                Serial.println(card.uid);
                Serial.print("Location  : ");
                Serial.println(card.location);
                Serial.print("Room No   : ");
                Serial.println(card.roomNumber);

                if (!card.found)
                {
                    strncpy(robotState.statusMessage, "UNKNOWN CARD", sizeof(robotState.statusMessage) - 1);
                    robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';
                }

                // State transitions based on scanned tags
                if (currentState == STATE_SEARCH_DOCK && card.found && strcmp(card.roomNumber, "DOCK") == 0)
                {
                    currentState = STATE_WAIT_ORDER;
                    strcpy(robotState.currentTask, "AT DOCK");
                    strncpy(robotState.statusMessage, "Docking station reached", sizeof(robotState.statusMessage) - 1);
                    robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

                    motorCmd.motion = MOTION_STOP;
                    motorCmd.leftSpeed = 0;
                    motorCmd.rightSpeed = 0;
                    xQueueSend(motorQueue, &motorCmd, 0);
                }
                else if (currentState == STATE_GO_TO_PHARMACY && card.found && strcmp(card.roomNumber, "PHARMACY") == 0)
                {
                    currentState = STATE_AT_PHARMACY_WAIT_DOOR;
                    strcpy(robotState.currentTask, "AT PHARMACY");
                    strncpy(robotState.statusMessage, "Pharmacy reached", sizeof(robotState.statusMessage) - 1);
                    robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

                    motorCmd.motion = MOTION_STOP;
                    motorCmd.leftSpeed = 0;
                    motorCmd.rightSpeed = 0;
                    xQueueSend(motorQueue, &motorCmd, 0);
                }
                else if (currentState == STATE_SEARCH_DESTINATION && card.found && strlen(destinationRoom) > 0 && strcmp(card.roomNumber, destinationRoom) == 0)
                {
                    currentState = STATE_UNLOADING_MEDICINE;
                    strcpy(robotState.currentTask, "ARRIVED");
                    strncpy(robotState.statusMessage, "Destination reached", sizeof(robotState.statusMessage) - 1);
                    robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

                    motorCmd.motion = MOTION_STOP;
                    motorCmd.leftSpeed = 0;
                    motorCmd.rightSpeed = 0;
                    xQueueSend(motorQueue, &motorCmd, 0);
                }
                else if (currentState == STATE_UNLOADING_MEDICINE && card.found && strcmp(card.roomNumber, "ADMIN") == 0)
                {
                    if (!adminDoorOpen)
                    {
                        controlDoor(DOOR_L_BOX, DOOR_OPEN);
                        adminDoorOpen = true;
                        strncpy(robotState.statusMessage, "Admin door opened", sizeof(robotState.statusMessage) - 1);
                        robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';
                    }
                    else
                    {
                        controlDoor(DOOR_L_BOX, DOOR_CLOSE);
                        adminDoorOpen = false;
                        currentState = STATE_RETURN_TO_DOCK;
                        strcpy(robotState.currentTask, "RETURN DOCK");
                        strncpy(robotState.statusMessage, "Returning to dock", sizeof(robotState.statusMessage) - 1);
                        robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';
                        destinationRoom[0] = '\0';

                        motorCmd.motion = MOTION_FORWARD;
                        motorCmd.leftSpeed = BASE_SPEED;
                        motorCmd.rightSpeed = BASE_SPEED;
                        xQueueSend(motorQueue, &motorCmd, 0);
                    }
                }
                else if (currentState == STATE_RETURN_TO_DOCK && card.found && strcmp(card.roomNumber, "DOCK") == 0)
                {
                    currentState = STATE_WAIT_ORDER;
                    strcpy(robotState.currentTask, "AT DOCK");
                    strncpy(robotState.statusMessage, "Docking station reached", sizeof(robotState.statusMessage) - 1);
                    robotState.statusMessage[sizeof(robotState.statusMessage) - 1] = '\0';

                    motorCmd.motion = MOTION_STOP;
                    motorCmd.leftSpeed = 0;
                    motorCmd.rightSpeed = 0;
                    xQueueSend(motorQueue, &motorCmd, 0);
                }

                Serial.println("═══════════════════════════════════");
            }

            xEventGroupClearBits(robotEventGroup, EVENT_NFC_DETECTED);
        }

        // ==========================
        // 5. LINE FOLLOW DRIVING STATES
        // ==========================
        if (currentState == STATE_SEARCH_DOCK ||
            currentState == STATE_GO_TO_PHARMACY ||
            currentState == STATE_SEARCH_DESTINATION ||
            currentState == STATE_RETURN_TO_DOCK)
        {
            if (xQueueReceive(encoderQueue, &encData, 0))
            {
                Serial.print("RPM L:");
                Serial.print(encData.leftRPM);
                Serial.print(" R:");
                Serial.println(encData.rightRPM);
            }
        }
        else if (currentState == STATE_WAIT_ORDER ||
                 currentState == STATE_AT_PHARMACY_WAIT_DOOR ||
                 currentState == STATE_UNLOADING_MEDICINE)
        {
            motorCmd.motion = MOTION_STOP;
            motorCmd.leftSpeed = 0;
            motorCmd.rightSpeed = 0;
            xQueueSend(motorQueue, &motorCmd, 0);
        }

        // ==========================
        // 6. STOP STATE
        // ==========================
        if (currentState == STATE_STOP)
        {
            Serial.println("Robot STOPPED");
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if (events & EVENT_OBSTACLE_DETECTED)
        {
            motorCmd.motion = MOTION_STOP;
            motorCmd.leftSpeed = 0;
            motorCmd.rightSpeed = 0;
            xQueueSend(motorQueue, &motorCmd, 0);
            Serial.println("Obstacle Detected");
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
