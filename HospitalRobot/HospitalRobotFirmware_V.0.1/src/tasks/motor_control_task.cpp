#include <Arduino.h>

#include "queue_manager.h"
#include "motor_driver.h"

void MotorControlTask(void *pvParameters)
{
    MotorCommand_t cmd;

    motorInit();

    while (1)
    {
        if (xQueueReceive(
                motorQueue,
                &cmd,
                portMAX_DELAY))
        {
            switch (cmd.motion)
            {
                case MOTION_FORWARD:
                    moveForward(
                        cmd.leftSpeed,
                        cmd.rightSpeed);
                    break;

                case MOTION_BACKWARD:
                    moveBackward(
                        cmd.leftSpeed,
                        cmd.rightSpeed);
                    break;

                case MOTION_LEFT:
                    turnLeft(cmd.leftSpeed);
                    break;

                case MOTION_RIGHT:
                    turnRight(cmd.rightSpeed);
                    break;

                case MOTION_STOP:
                default:
                    stopMotors();
                    break;
            }
        }
    }
}