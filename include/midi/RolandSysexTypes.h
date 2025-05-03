#ifndef __ROLAND_SYSEX_H__
#define __ROLAND_SYSEX_H__

#include <Arduino.h>

namespace MIDI {

    #define ROLAND_SYSEX_MAN_CODE   0x41
    #define ROLAND_SYSEX_HDR_SIZE   6
    #define ROLAND_SYSEX_CMD_READ   0x11
    #define ROLAND_SYSEX_CMD_WRITE  0x12

    typedef struct {        
        volatile uint8_t hsb;    // highest significant byte
        volatile uint8_t msb;    // middle/medium significant byte
        volatile uint8_t lsb;    // least significant byte

    } RolandAddr;

    typedef enum : uint8_t{
        masquerade = 0,
        base128 = 1
    } Encoding;

    /**
     * Defines an info set for a given address
     */
    typedef struct {        
        uint16_t recordCount;       // number of records being sent/received
        Encoding recordEncoding;    // information about the encoding
    } AddressInfo;

    /**
     * Defines the record info which is required to 
     * read or send some sysex message.
     */
    typedef struct {
        uint16_t index;     // the record number, this info belongs to.
        uint16_t size;      // the size of the record
        RolandAddr addr;    // the address of the record.
    } RecordInfo;

    typedef struct {
        volatile uint8_t device;
        volatile uint8_t model;
        volatile uint8_t cmd;
        
        RolandAddr addr;
    } RolandSysexHdr;

}

#endif // __ROLAND_SYSEX_H__