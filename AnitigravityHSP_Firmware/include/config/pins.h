#ifndef CONFIG_PINS_H
#define CONFIG_PINS_H

#include <Arduino.h>

// Motor Driver (L298N)
// Left Motor
constexpr uint8_t PIN_MOTOR_L_PWM  = 25; // ENA
constexpr uint8_t PIN_MOTOR_L_DIR1 = 26; // IN1
constexpr uint8_t PIN_MOTOR_L_DIR2 = 27; // IN2

// Right Motor
constexpr uint8_t PIN_MOTOR_R_PWM  = 33; // ENB
constexpr uint8_t PIN_MOTOR_R_DIR1 = 32; // IN3
constexpr uint8_t PIN_MOTOR_R_DIR2 = 14; // IN4

// Encoders
constexpr uint8_t PIN_ENCODER_L = 34; // Interrupt-driven
constexpr uint8_t PIN_ENCODER_R = 35; // Interrupt-driven

// Line Sensors (IR Sensors)
constexpr uint8_t PIN_LINE_L = 36; // Left IR
constexpr uint8_t PIN_LINE_R = 39; // Right IR

// Shared I2C Bus (VL53L0X, MPU6050, SSD1306)
constexpr uint8_t PIN_I2C_SDA = 21;
constexpr uint8_t PIN_I2C_SCL = 22;

// VL53L0X Distance Sensor
constexpr uint8_t VL53L0X_I2C_ADDRESS = 0x29;

// NFC Reader (PN532) SPI Interface
constexpr uint8_t PIN_NFC_SCK  = 18;
constexpr uint8_t PIN_NFC_MISO = 19;
constexpr uint8_t PIN_NFC_MOSI = 23;
constexpr uint8_t PIN_NFC_SS   = 5;
constexpr uint8_t PIN_NFC_IRQ  = 4;

// User Buttons
constexpr uint8_t PIN_BTN_LEFT   = 16; // Moved from 12 to resolve GPIO34 conflict
constexpr uint8_t PIN_BTN_SELECT = 13;
constexpr uint8_t PIN_BTN_RIGHT  = 15;

// Battery Monitoring
constexpr uint8_t PIN_BATTERY_ADC = 12; // Moved from 34 to resolve GPIO34 conflict

// Buzzer
constexpr uint8_t PIN_BUZZER = 2;

#endif // CONFIG_PINS_H
