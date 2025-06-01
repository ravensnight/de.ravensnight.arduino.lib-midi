#ifndef __ROLAND_SYSEX_H__
#define __ROLAND_SYSEX_H__

#include <Arduino.h>

#include <midi/RolandSysexAddr.h>

namespace ravensnight::midi {

    #define ROLAND_SYSEX_MAN_CODE   0x41
    #define ROLAND_SYSEX_HDR_SIZE   6
    #define ROLAND_SYSEX_CMD_READ   0x11
    #define ROLAND_SYSEX_CMD_WRITE  0x12

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
        RolandSysexAddr addr;    // the address of the record.
    } RecordInfo;
}

#endif // __ROLAND_SYSEX_H__