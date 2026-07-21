#ifndef CONFIG_H
#define CONFIG_H

// ====================================
// HARDWARE PIN DEFINITIONS
// ====================================
// ==========================
// SERVO MOTOR PINS (Door Control)
// ==========================
#define SERVO_L_BOX_DOOR     32  // Large box door servo
#define SERVO_M_BOX_DOOR     12  // Medium box door servo

// Servo angle definitions
// These values are intentionally reversed to match the physical door motion.
// On this hardware, the open position is the larger angle and the closed position is the smaller angle.
#define DOOR_OPEN_ANGLE      10
#define DOOR_CLOSE_ANGLE     150
// ==========================
// MOTOR DRIVER PINS
// ==========================

#define LEFT_MOTOR_PWM       25
#define LEFT_MOTOR_IN1       26
#define LEFT_MOTOR_IN2       27

#define RIGHT_MOTOR_PWM      17
#define RIGHT_MOTOR_IN1      16
#define RIGHT_MOTOR_IN2      4

// ==========================
// ENCODER PINS
// ==========================

//#define LEFT_ENCODER_A       34
//#define RIGHT_ENCODER_A      35

// ==========================
// IR LINE SENSOR PINS
// ==========================

#define IR_LEFT              34
#define IR_RIGHT             2

// ==========================
// RC522 RFID MODULE PINS (SPI)
// ==========================
#define RFID_MOSI    23
#define RFID_MISO    19
#define RFID_SCK     18
#define RFID_SS      5   // Chip Select
#define RFID_RST     13  // Reset pin

// ==========================
// ULTRASONIC SENSOR PINS
// ==========================
//#define TRIGGER_PIN          15
//#define ECHO_PIN             4

// Shared I2C Bus

#define I2C_SDA 21
#define I2C_SCL 22
// TOF Shutdown Pin (optional)
#define TOF_XSHUT 35

// ====================================
// COMMUNICATION CONFIG
// ====================================
#define WIFI_SSID              "Spider"
#define WIFI_PASSWORD          "12Aberlour"
#define WEBSOCKET_HOST         "192.168.178.113"
#define WEBSOCKET_PORT         3000
#define WEBSOCKET_PATH         "/ws/robot-chat"
#define ROBOT_ID               "RBT-001"
#define WEBSOCKET_RECONNECT    5000
#define HEARTBEAT_INTERVAL     5000
#define WifiStatus

// ====================================
// PWM CONFIGURATION
// ====================================
#define PWM_FREQ               1000
#define PWM_RESOLUTION         8
#define LEFT_PWM_CH            0
#define RIGHT_PWM_CH           1

// ====================================
// MOTOR PARAMETERS
// ====================================
#define MAX_SPEED              175
#define BASE_SPEED             170

// ====================================
// RTOS CONFIGURATION
// ====================================
#define STACK_SIZE_SMALL       2048
#define STACK_SIZE_MEDIUM      4096
#define STACK_SIZE_LARGE       8192

#define PRIORITY_MOTOR         4
#define PRIORITY_CONTROL       5
#define PRIORITY_ENCODER       3
#define PRIORITY_LINEFOLLOW    4
#define PRIORITY_NFC           2
#define PRIORITY_COMM          3
#define PRIORITY_TOF           3

#define MOTOR_QUEUE_SIZE       10
#define SENSOR_QUEUE_SIZE      10
#define MESSAGE_QUEUE_SIZE     10

#endif