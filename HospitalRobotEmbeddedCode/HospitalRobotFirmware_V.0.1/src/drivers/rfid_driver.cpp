#include "rfid_driver.h"
#include "Config.h"
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Create RFID reader instance
MFRC522 rfid(RFID_SS, RFID_RST);

bool rfidInit()
{
    // Initialize SPI bus
    SPI.begin(RFID_SCK, RFID_MISO, RFID_MOSI, RFID_SS);
    
    // Initialize RFID reader
    rfid.PCD_Init();
    
    // Check if RFID module is connected
    byte version = rfid.PCD_ReadRegister(MFRC522::VersionReg);
    
    if (version == 0x00 || version == 0xFF)
    {
        Serial.println("❌ RC522 RFID Reader not found!");
        return false;
    }
    
    Serial.print("✅ RC522 RFID Reader initialized. Firmware version: ");
    Serial.println(version, HEX);
    
    return true;
}

bool readRFID(char *uidBuffer, size_t bufferSize)
{
    // Check for new cards
    if (!rfid.PICC_IsNewCardPresent())
    {
        return false;
    }
    
    // Select one of the cards
    if (!rfid.PICC_ReadCardSerial())
    {
        return false;
    }
    
    // Get the UID of the card
    String uidString = "";
    for (byte i = 0; i < rfid.uid.size; i++)
    {
        // Convert byte to hex string
        if (rfid.uid.uidByte[i] < 0x10)
        {
            uidString += "0";
        }
        uidString += String(rfid.uid.uidByte[i], HEX);
    }
    
    // Convert to uppercase
    uidString.toUpperCase();
    
    // Copy to buffer with bounds checking
    if (bufferSize == 0)
    {
        return false;
    }
    
    strncpy(uidBuffer, uidString.c_str(), bufferSize - 1);
    uidBuffer[bufferSize - 1] = '\0';
    
    return true;
}

// ========================================
// RFID Data Lookup Function
// ========================================

void getRoomInfoFromUID(const char *uid, RFIDResult_t *result)
{
    // Initialize result as not found
    result->found = false;
    result->roomNumber = "UNKNOWN";
    strcpy(result->uid, uid);
    strcpy(result->location, "Unknown");
    
    // Search through RFID database
    for (int i = 0; i < RFID_DATABASE_SIZE; i++)
    {
        // Case-insensitive comparison
        if (strcasecmp(uid, RFID_DATABASE[i].uid) == 0)
        {
            result->found = true;
            result->roomNumber = RFID_DATABASE[i].roomNumber;
            strcpy(result->location, RFID_DATABASE[i].location);
            return;
        }
    }
}
