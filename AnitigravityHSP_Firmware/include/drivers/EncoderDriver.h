#ifndef DRIVERS_ENCODER_DRIVER_H
#define DRIVERS_ENCODER_DRIVER_H

#include <Arduino.h>

class EncoderDriver {
public:
    static EncoderDriver& getInstance();

    // Prevent copying
    EncoderDriver(const EncoderDriver&) = delete;
    EncoderDriver& operator=(const EncoderDriver&) = delete;

    void init();
    void getTicks(int32_t& left, int32_t& right);
    void reset();

    // Static ISRs (must be public to be visible to the ISR linkage)
    static void IRAM_ATTR handleLeftISR();
    static void IRAM_ATTR handleRightISR();

private:
    EncoderDriver() = default;
    ~EncoderDriver() = default;

    static volatile int32_t s_leftTicks;
    static volatile int32_t s_rightTicks;
    static portMUX_TYPE s_mux;
};

#endif // DRIVERS_ENCODER_DRIVER_H
