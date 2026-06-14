#include <Arduino.h> // Or native ESP-IDF headers if preferred
#include "USS.h"
#include "Config.h" // For pin definitions
#include "Config.h"

// Global variables shared between ISR and Nav Task (marked volatile)
volatile int64_t echo_start_time = 0;
volatile int64_t echo_end_time = 0;
volatile float calculated_distance_cm = -1.0; 
volatile bool new_distance_ready = false;

// ==========================================
// Echo Pin Interrupt Service Routine (ISR)
// ==========================================
void IRAM_ATTR echo_isr() {
    int64_t current_time = esp_timer_get_time(); // Get time in microseconds

    if (digitalRead(ECHO_PIN) == HIGH) {
        // Echo pin just went HIGH -> sound wave was sent out
        echo_start_time = current_time;
    } else {
        // Echo pin just went LOW -> sound wave bounced back
        echo_end_time = current_time;
        int64_t flight_time = echo_end_time - echo_start_time;
        
        // Speed of sound is ~0.0343 cm per microsecond. Divide by 2 for round-trip.
        calculated_distance_cm = (float)flight_time * 0.0343 / 2.0;
        new_distance_ready = true;
    }
}

// ==========================================
// Initialization Functions
// ==========================================
void setup_ultrasonic() {
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIGGER_PIN, LOW);

    // Attach interrupt to fire on BOTH rising and falling edges
    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echo_isr, CHANGE);
}

// ==========================================
// Inside your Navigation Loop (Core 1)
// ==========================================
void vNavTask(void *pvParameters) {
    setup_ultrasonic();
    float local_distance = 400.0; // Default to clear path

    for (;;) {
        // 1. Fire a non-blocking trigger pulse (takes exactly 10 microseconds)
        digitalWrite(TRIGGER_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGGER_PIN, LOW);
        Serial.println("Trigger pulse sent");

        // 2. Instead of waiting, we immediately look at what the LAST completed 
        // interrupt cycle calculated.
        if (new_distance_ready) {
            local_distance = calculated_distance_cm;
            Serial.print("New distance measured: ");
            Serial.print(local_distance);
            Serial.println(" cm");
            new_distance_ready = false; // Reset flag
        }

        // 3. Act on the data
        if (local_distance < 15.0 && local_distance > 1.0) {
            Serial.println("Obstacle detected!");
            // Obstacle handling: Stop or veer away!
        } else {
            // Line tracking algorithm goes here...
        }

        // 4. Sleep for the rest of your 10ms cycle
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}