#include <Arduino.h>

#include "Config.h"
#include "queue_manager.h"
#include "structs.h"

#define ENCODER_PPR  20   // pulses per revolution (CHANGE based on your encoder)
#define WHEEL_CIRCUMFERENCE  0.21  // meters (example: 21 cm wheel circumference)

volatile long leftPulseCount = 0;
volatile long rightPulseCount = 0;

// ==========================
// ISR FUNCTIONS
// ==========================

void IRAM_ATTR leftEncoderISR()
{
    leftPulseCount++;
}

void IRAM_ATTR rightEncoderISR()
{
    rightPulseCount++;
}

// ==========================
// Encoder Task
// ==========================

void EncoderTask(void *pvParameters)
{
    EncoderData_t data;

    long lastLeftCount = 0;
    long lastRightCount = 0;

    const float sampleTime = 0.1; // 100ms

    // ==========================
    // Setup interrupts
    // ==========================
    pinMode(LEFT_ENCODER_A , INPUT);
    pinMode(RIGHT_ENCODER_A, INPUT);

    attachInterrupt(digitalPinToInterrupt(LEFT_ENCODER_A), leftEncoderISR, RISING);
    attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODER_A), rightEncoderISR, RISING);

    while (1)
    {
        // ==========================
        // Copy safely (atomic read)
        // ==========================
        noInterrupts();
        long leftCount = leftPulseCount;
        long rightCount = rightPulseCount;
        interrupts();

        // ==========================
        // Calculate delta
        // ==========================
        long deltaLeft = leftCount - lastLeftCount;
        long deltaRight = rightCount - lastRightCount;

        lastLeftCount = leftCount;
        lastRightCount = rightCount;

        // ==========================
        // Convert to RPM
        // ==========================
        float leftRPM = (deltaLeft * (60.0 / ENCODER_PPR)) / sampleTime;
        float rightRPM = (deltaRight * (60.0 / ENCODER_PPR)) / sampleTime;

        // ==========================
        // Fill struct
        // ==========================
        data.leftTicks = leftCount;
        data.rightTicks = rightCount;

        data.leftRPM = leftRPM;
        data.rightRPM = rightRPM;

        // ==========================
        // Send to queue
        // ==========================
        xQueueSend(encoderQueue, &data, 0);

        // ==========================
        // Debug
        // ==========================
        Serial.print("L ticks:");
        Serial.print(leftCount);
        Serial.print(" R ticks:");
        Serial.print(rightCount);

        Serial.print(" | L RPM:");
        Serial.print(leftRPM);
        Serial.print(" R RPM:");
        Serial.println(rightRPM);

        // ==========================
        // Loop delay
        // ==========================
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}