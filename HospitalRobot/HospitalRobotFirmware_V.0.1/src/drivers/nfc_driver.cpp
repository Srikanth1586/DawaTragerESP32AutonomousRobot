#include "nfc_driver.h"
#include "Config.h"
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>


// IMPORTANT: use I2C explicitly
//Adafruit_PN532 nfc(&Wire);
Adafruit_PN532 nfc(NFC_IRQ, NFC_RESET);
//Adafruit_PN532 nfc(&Wire);
bool nfcInit()
{
    // Start I2C using your pins
    Wire.begin(NFC_SDA, NFC_SCL);

    nfc.begin();

    uint32_t version = nfc.getFirmwareVersion();

    if (!version)
    {
        Serial.println("❌ PN532 not found!");
        return false;
    }

    Serial.print("PN532 Found. Firmware: ");
    Serial.println(version);

    nfc.SAMConfig();  // enable reading mode

    return true;
}

bool readNFC(char *uidBuffer)
{
    uint8_t uid[7];
    uint8_t uidLength;

    bool success = nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength);

    if (!success)
        return false;

    // Convert UID → HEX string
    for (int i = 0; i < uidLength; i++)
    {
        sprintf(uidBuffer + (i * 2), "%02X", uid[i]);
    }

    uidBuffer[uidLength * 2] = '\0';

    return true;
}