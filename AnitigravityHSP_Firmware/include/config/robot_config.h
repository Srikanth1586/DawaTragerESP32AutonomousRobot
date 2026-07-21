#ifndef CONFIG_ROBOT_CONFIG_H
#define CONFIG_ROBOT_CONFIG_H

#include <Arduino.h>

// Robot Operational States
enum class RobotState : uint8_t {
    IDLE = 0,
    LINE_FOLLOWING,
    CHECKPOINT_DETECTED,
    OBSTACLE_AVOIDANCE,
    CHARGING,
    EMERGENCY_STOP
};

// Return a string representation of the state for debugging/display
inline const char* robotStateToString(RobotState state) {
    switch (state) {
        case RobotState::IDLE:                return "IDLE";
        case RobotState::LINE_FOLLOWING:      return "LINE_FOLLOWING";
        case RobotState::CHECKPOINT_DETECTED: return "CHECKPOINT";
        case RobotState::OBSTACLE_AVOIDANCE:  return "OBSTACLE_AVOID";
        case RobotState::CHARGING:            return "CHARGING";
        case RobotState::EMERGENCY_STOP:      return "EMERGENCY_STOP";
        default:                              return "UNKNOWN";
    }
}

// PWM Configuration (for LEDC on ESP32)
constexpr uint32_t MOTOR_PWM_FREQ = 20000; // 20 kHz PWM
constexpr uint8_t MOTOR_PWM_RES   = 8;     // 8-bit resolution (0-255)
constexpr uint8_t LEDC_CHANNEL_L  = 0;
constexpr uint8_t LEDC_CHANNEL_R  = 1;

// Motor speed limit
constexpr int16_t MAX_MOTOR_SPEED = 255;
constexpr int16_t BASE_MOTOR_SPEED = 120; // Default base speed for line following

// Wheel & Encoder Parameters
constexpr float WHEEL_DIAMETER_M = 0.065f;           // 65mm wheel
constexpr float WHEEL_CIRCUMFERENCE_M = WHEEL_DIAMETER_M * 3.14159f;
constexpr float ENCODER_CPR = 20.0f;                // Counts Per Revolution (adjust for specific encoder)
constexpr float GEAR_RATIO = 30.0f;                 // Gear ratio (adjust for specific motor)
constexpr float ENCODER_PULSES_PER_REV = ENCODER_CPR * GEAR_RATIO;

// Line Follower PID Tuning
constexpr float LINE_KP = 1.8f;
constexpr float LINE_KI = 0.01f;
constexpr float LINE_KD = 0.5f;

// Motor Velocity PID Tuning (Speed Control)
constexpr float MOTOR_KP = 1.2f;
constexpr float MOTOR_KI = 0.1f;
constexpr float MOTOR_KD = 0.05f;

// Sensor Parameters
constexpr uint16_t OBSTACLE_DISTANCE_THRESHOLD_MM = 300; // 300 mm obstacle threshold
constexpr uint16_t LOST_LINE_TIMEOUT_MS = 2000;          // Time before declaring lost line state

// Battery Monitoring Settings (Using 12-bit ADC on ESP32: 0-4095)
constexpr float BATTERY_ADC_REF_V = 3.3f;
constexpr float BATTERY_R1 = 10000.0f;                  // 10k ohm resistor
constexpr float BATTERY_R2 = 10000.0f;                  // 10k ohm resistor (voltage divider ratio 2.0)
constexpr float BATTERY_DIVIDER_RATIO = (BATTERY_R1 + BATTERY_R2) / BATTERY_R2;
constexpr float BATTERY_MAX_V = 12.6f;                   // 3S LiPo Max voltage
constexpr float BATTERY_MIN_V = 9.9f;                    // 3S LiPo Min voltage (cutoff)
constexpr float BATTERY_LOW_THRESHOLD_PCT = 20.0f;       // Battery low event threshold (%)

// Default Network Configuration
constexpr char DEFAULT_WIFI_SSID[] = "Hospital_Robot_AP";
constexpr char DEFAULT_WIFI_PASS[] = "SecureDelivery2026";
constexpr char DEFAULT_WS_SERVER[] = "192.168.1.100";
constexpr uint16_t DEFAULT_WS_PORT  = 8080;
constexpr char DEFAULT_WS_PATH[]   = "/robot";

// Robot Identity
constexpr char ROBOT_ID[] = "ROBOT-HSP-01";

#endif // CONFIG_ROBOT_CONFIG_H
