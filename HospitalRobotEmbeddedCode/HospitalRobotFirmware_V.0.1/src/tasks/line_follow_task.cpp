#include <Arduino.h>
#include "Config.h"
#include "queue_manager.h"
#include "structs.h"
#include "config.h"
#include "robot_state.h"

void LineFollowTask(void *pvParameters)
{
    MotorCommand_t cmd;

    float Kp = 50.0;     // tuning parameter
    int baseSpeed = BASE_SPEED;

    while (1)
    {
        // Only run the line follower in active motion states
        if (!(currentState == STATE_SEARCH_DOCK ||
              currentState == STATE_GO_TO_PHARMACY ||
              currentState == STATE_SEARCH_DESTINATION ||
              currentState == STATE_RETURN_TO_DOCK))
        {
            cmd.motion = MOTION_STOP;
            cmd.leftSpeed = 0;
            cmd.rightSpeed = 0;
            xQueueSend(motorQueue, &cmd, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // =====================
        // 1. Read sensors
        // =====================
        int L = digitalRead(IR_LEFT);
        int R = digitalRead(IR_RIGHT);

        // =====================
        // 2. Convert logic
        // assume BLACK line = 0
        // =====================
        L = (L == 0) ? 1 : 0;
        R = (R == 0) ? 1 : 0;

        // =====================
        // 3. Compute error
        // =====================
        int error = R - L;
        float correction = Kp * error;

        int leftSpeed  = baseSpeed - correction;
        int rightSpeed = baseSpeed + correction;

        leftSpeed  = constrain(leftSpeed, 0, 200);
        rightSpeed = constrain(rightSpeed, 0, 200);
        // =====================
        // 4. Lost line handling
        // =====================
        if (L == 0 && R == 0)
        {
            cmd.motion = MOTION_STOP;
            cmd.leftSpeed = 0;
            cmd.rightSpeed = 0;

            xQueueSend(motorQueue, &cmd, 0);

            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        else if (L == 1 && R == 0)
        {
           
            cmd.motion = MOTION_LEFT;
            cmd.leftSpeed = 100;
            cmd.rightSpeed = 200;
            //Serial.println("Turning Left");
            xQueueSend(motorQueue, &cmd, 0);

        }
        else if (L == 0 && R == 1)
        {
            cmd.motion = MOTION_RIGHT;
            cmd.leftSpeed = 200;
            cmd.rightSpeed = 100;
            //Serial.println("Turning Right");
            xQueueSend(motorQueue, &cmd, 0);
        }
        else if (L == 1 && R == 1)
        {
            cmd.motion = MOTION_FORWARD;
            cmd.leftSpeed = leftSpeed;
            cmd.rightSpeed = rightSpeed;
            //Serial.println("Going Forward");
        xQueueSend(motorQueue, &cmd, 0);
        }

        // =====================
        // 5. Correction
        // =====================


        // =====================
        // 6. Motor command
        // =====================

        // =====================
        // 7. Debug
        // =====================
     /*   Serial.print("L:");
        Serial.print(L);
        Serial.print(" R:");
        Serial.print(R);
        Serial.print(" Err:");
        Serial.print(error);
        Serial.print(" LS:");
        Serial.print(leftSpeed);
        Serial.print(" RS:");
        Serial.println(rightSpeed);*/

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
