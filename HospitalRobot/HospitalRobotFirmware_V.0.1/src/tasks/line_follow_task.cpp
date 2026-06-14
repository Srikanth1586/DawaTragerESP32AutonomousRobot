#include <Arduino.h>
#include "Config.h"
#include "queue_manager.h"
#include "structs.h"
#include "config.h"

void LineFollowTask(void *pvParameters)
{
    MotorCommand_t cmd;

    float Kp = 80.0;     // tuning parameter
    int baseSpeed = BASE_SPEED;

    while (1)
    {
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

        // =====================
        // 4. Lost line handling
        // =====================
        if (L == 0 && R == 0)
        {
            cmd.motion = MOTION_STOP;
            cmd.leftSpeed = 0;
            cmd.rightSpeed = 0;

            xQueueSend(motorQueue, &cmd, 0);

            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // =====================
        // 5. Correction
        // =====================
        float correction = Kp * error;

        int leftSpeed  = baseSpeed - correction;
        int rightSpeed = baseSpeed + correction;

        leftSpeed  = constrain(leftSpeed, 0, 255);
        rightSpeed = constrain(rightSpeed, 0, 255);

        // =====================
        // 6. Motor command
        // =====================
        cmd.motion = MOTION_FORWARD;
        cmd.leftSpeed = leftSpeed;
        cmd.rightSpeed = rightSpeed;

        xQueueSend(motorQueue, &cmd, 0);

        // =====================
        // 7. Debug
        // =====================
        Serial.print("L:");
        Serial.print(L);
        Serial.print(" R:");
        Serial.print(R);
        Serial.print(" Err:");
        Serial.print(error);
        Serial.print(" LS:");
        Serial.print(leftSpeed);
        Serial.print(" RS:");
        Serial.println(rightSpeed);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
