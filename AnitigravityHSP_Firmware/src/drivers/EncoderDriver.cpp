#include "drivers/EncoderDriver.h"
#include "config/pins.h"

// Define static members
volatile int32_t EncoderDriver::s_leftTicks = 0;
volatile int32_t EncoderDriver::s_rightTicks = 0;
portMUX_TYPE EncoderDriver::s_mux = portMUX_INITIALIZER_UNLOCKED;

EncoderDriver& EncoderDriver::getInstance() {
    static EncoderDriver instance;
    return instance;
}

void EncoderDriver::init() {
    // Configure pins as input (External pull-ups are assumed on the robot PCB)
    pinMode(PIN_ENCODER_L, INPUT);
    pinMode(PIN_ENCODER_R, INPUT);

    // Register interrupts on RISING edge
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_L), handleLeftISR, RISING);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_R), handleRightISR, RISING);

    reset();
}

void EncoderDriver::getTicks(int32_t& left, int32_t& right) {
    portENTER_CRITICAL(&s_mux);
    left = s_leftTicks;
    right = s_rightTicks;
    portEXIT_CRITICAL(&s_mux);
}

void EncoderDriver::reset() {
    portENTER_CRITICAL(&s_mux);
    s_leftTicks = 0;
    s_rightTicks = 0;
    portEXIT_CRITICAL(&s_mux);
}

void IRAM_ATTR EncoderDriver::handleLeftISR() {
    portENTER_CRITICAL_ISR(&s_mux);
    s_leftTicks++;
    portEXIT_CRITICAL_ISR(&s_mux);
}

void IRAM_ATTR EncoderDriver::handleRightISR() {
    portENTER_CRITICAL_ISR(&s_mux);
    s_rightTicks++;
    portEXIT_CRITICAL_ISR(&s_mux);
}
