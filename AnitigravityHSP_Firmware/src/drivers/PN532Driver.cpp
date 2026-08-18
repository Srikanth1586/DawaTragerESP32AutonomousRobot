#include "drivers/PN532Driver.h"
#include "config/pins.h"

PN532Driver::PN532Driver() : m_nfc(PIN_NFC_SCK, PIN_NFC_MISO, PIN_NFC_MOSI, PIN_NFC_SS) {}

bool PN532Driver::init() {
    m_nfc.begin();

    uint32_t versiondata = m_nfc.getFirmwareVersion();
    if (!versiondata) {
        m_initialized = false;
        return false;
    }

    // Configure board to read RFID tags
    m_nfc.SAMConfig();
    m_initialized = true;
    return true;
}

bool PN532Driver::readTag(char* stationID, size_t maxLen) {
    if (!m_initialized || stationID == nullptr || maxLen == 0) {
        return false;
    }

    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

    // Try to read passive target ID (timeout 50ms to keep NFC task non-blocking)
    success = m_nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);

    if (success) {
        // Format the UID as a string
        char tempID[32];
        if (uidLength == 4) {
            snprintf(tempID, sizeof(tempID), "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
        } else {
            snprintf(tempID, sizeof(tempID), "%02X%02X%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
        }

        // Map known UIDs to human-readable names
        if (strcmp(tempID, "A1B2C3D4") == 0 || strcmp(tempID, "04A1B2C3D4E5F6") == 0) {
            strncpy(stationID, "Station A", maxLen - 1);
        } else if (strcmp(tempID, "E1F2G3H4") == 0) {
            strncpy(stationID, "Station B", maxLen - 1);
        } else if (strcmp(tempID, "12345678") == 0) {
            strncpy(stationID, "Docking Stn", maxLen - 1);
        } else {
            // General tag
            snprintf(stationID, maxLen, "Tag: %s", tempID);
        }
        stationID[maxLen - 1] = '\0';
        return true;
    }

    return false;
}
