#ifndef DRIVERS_PN532_DRIVER_H
#define DRIVERS_PN532_DRIVER_H

#include <Arduino.h>
#include <Adafruit_PN532.h>

class PN532Driver {
public:
    PN532Driver();
    ~PN532Driver() = default;

    bool init();
    bool readTag(char* stationID, size_t maxLen);

private:
    Adafruit_PN532 m_nfc;
    bool m_initialized = false;
};

#endif // DRIVERS_PN532_DRIVER_H
