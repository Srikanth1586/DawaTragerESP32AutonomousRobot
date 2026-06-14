#include <Arduino.h>

#include "queue_manager.h"
#include "event_manager.h"
#include "structs.h"
#include "system_state.h"
#include "Config.h"
#include "Comm.h"
#include "robot_state.h"

RobotMode currentState = STATE_LINE_FOLLOW;

MotorCommand_t motorCmd;
EncoderData_t encData;
NFCData_t nfcData;
ToFData_t tofData;

// ==========================
// CONTROL TASK
// ==========================

void ControlTask(void *pvParameters)
{
    EventBits_t events;
    RobotCommand_t cmd;

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

            motorCmd.motion = MOTION_STOP;
            motorCmd.leftSpeed = 0;
            motorCmd.rightSpeed = 0;

            xQueueSend(motorQueue, &motorCmd, 0);
        }
        // ==========================
        // 2. PRIORITY: COMMAND RECEIVED
        // ==========================
        if(xQueueReceive(
                commandQueue,
                &cmd,
                portMAX_DELAY))
        {
            Serial.println("CONTROL TASK");

            Serial.print("Command : ");
            Serial.println(cmd.command);

            Serial.print("Room : ");
            Serial.println(cmd.roomNumber);

            if(cmd.command == "go_to_pharmacy")
            {
                // Start pharmacy navigation
            }
            else if(cmd.command == "deliver_medicine")
            {
                // Navigate to room
            }
        }

        // ==========================
        // 3. NFC EVENT HANDLING
        // ==========================
        if (events & EVENT_NFC_DETECTED)
        {
            if (xQueueReceive(nfcQueue, &nfcData, 0))
            {
                Serial.print("NFC UID: ");
                Serial.println(nfcData.uid);

                currentState = STATE_NFC_ACTION;

                // Example behavior:
                // stop at station for 2 sec
                motorCmd.motion = MOTION_STOP;
                motorCmd.leftSpeed = 0;
                motorCmd.rightSpeed = 0;

                xQueueSend(motorQueue, &motorCmd, 0);

                vTaskDelay(pdMS_TO_TICKS(2000));

                // return to line following
                currentState = STATE_LINE_FOLLOW;
            }

            xEventGroupClearBits(robotEventGroup, EVENT_NFC_DETECTED);
        }

        // ==========================
        // 4. LINE FOLLOW MODE
        // ==========================
        if (currentState == STATE_LINE_FOLLOW)
        {
            // optional encoder feedback read
            if (xQueueReceive(encoderQueue, &encData, 0))
            {
                // You can later use this for PID correction
                Serial.print("RPM L:");
                Serial.print(encData.leftRPM);
                Serial.print(" R:");
                Serial.println(encData.rightRPM);
            }

            // ControlTask does NOT generate motor commands here
            // LineFollowTask already sends motor commands
        }

        // ==========================
        // 5. STOP STATE
        // ==========================
        if (currentState == STATE_STOP)
        {
            Serial.println("Robot STOPPED");

            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        if(events & EVENT_OBSTACLE_DETECTED)
        {
            motorCmd.motion = MOTION_STOP;
            motorCmd.leftSpeed = 0;
            motorCmd.rightSpeed = 0;

        xQueueSend(
            motorQueue,
            &motorCmd,
            0);
            Serial.println(
            "Obstacle Detected");
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
