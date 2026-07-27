#ifndef RFID_DATA_H
#define RFID_DATA_H

#include <Arduino.h>

// ========================================
// RFID Card UID Mapping
// ========================================

typedef struct
{
    const char* uid;
    const char* location;
    const char* roomNumber;
} RFIDCard_t;

// RFID Card Database
static const RFIDCard_t RFID_DATABASE[] = {
    {"04AE3BBFCC2A81", "PHARMACY", "PHARMACY"},
    {"048F33BFCC2A81", "DOCK", "DOCK"},
    {"04832CBFCC2A81", "ROOM-1", "A101"},
    {"048122BFCC2A81", "ROOM-2", "A102"},
    {"046D1BBFCC2A81", "ROOM-3", "A103"},
    {"04281BBFCC2A81", "ROOM-4", "A104"},
    {"04FFFBBECC2A81", "ROOM-5", "A105"},
    {"04EDF1BECC2A81", "ROOM-8", "A108"},
    {"04E9F1BECC2A81", "ROOM-7", "A107"},
    {"04E5F1BECC2A81", "ROOM-6", "A106"},
    {"04ECF1BECC2A81", "ROOM-9", "A109"},
    {"0411FCBECC2A81", "ROOM-10", "A110"},
    {"E3146A00", "ADMIN", "ADMIN"}
};

#define RFID_DATABASE_SIZE (sizeof(RFID_DATABASE) / sizeof(RFIDCard_t))

// ========================================
// RFID Result Structure
// ========================================

typedef struct
{
    char uid[32];
    char location[30];
    const char* roomNumber;
    bool found;
} RFIDResult_t;

extern RFIDResult_t lastScannedCard;

// ========================================
// RFID Lookup Function
// ========================================

static inline RFIDResult_t getRFIDInfo(const char *uid)
{
    RFIDResult_t result;

    strncpy(result.uid, uid, sizeof(result.uid) - 1);
    result.uid[sizeof(result.uid) - 1] = '\0';
    strncpy(result.location, "Unknown", sizeof(result.location) - 1);
    result.location[sizeof(result.location) - 1] = '\0';
    result.roomNumber = "UNKNOWN";
    result.found = false;

    for (int i = 0; i < RFID_DATABASE_SIZE; i++)
    {
        if (strcmp(uid, RFID_DATABASE[i].uid) == 0)
        {
            strncpy(result.location, RFID_DATABASE[i].location, sizeof(result.location) - 1);
            result.location[sizeof(result.location) - 1] = '\0';
            result.roomNumber = RFID_DATABASE[i].roomNumber;
            result.found = true;
            break;
        }
    }

    return result;
}

#endif
