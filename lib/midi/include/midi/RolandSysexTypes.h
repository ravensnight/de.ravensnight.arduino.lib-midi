#ifndef __ROLAND_SYSEX_H__
#define __ROLAND_SYSEX_H__

#include <Arduino.h>

#include <midi/RolandSysexAddr.h>

namespace ravensnight::midi {

    #define ROLAND_SYSEX_MAN_CODE           0x41
    #define ROLAND_SYSEX_ADDR_SIZE          3
    #define ROLAND_SYSEX_HDR_SIZE           3 + ROLAND_SYSEX_ADDR_SIZE  

    typedef enum : uint8_t {
        undefined = 0x00,
        read = 0x11,
        write = 0x12, 
    } Command;

    typedef enum : uint8_t {
        masquerade = 0,
        base128 = 1
    } Encoding;

    /**
     * Defines an info set for a given address
     */
    typedef struct {
        uint16_t recordCount;       // number of records being sent/received
        Encoding recordEncoding;    // information about the encoding
        boolean  replyAck;          // send some acknowledgement message to sender when writing        
        RolandSysexAddr replyAddr;
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

    typedef enum : uint8_t {
        success = 0,    // finished with success
        next = 1,       // message accepted, expect more to send
        reject = 2,     // message has been rejected
        error = 3       // some internal error occured
    } Result;

    #define ACK_REPLY_SIZE  (ROLAND_SYSEX_ADDR_SIZE + 1)
    typedef struct {
        RolandSysexAddr addr;   // the address of the record to ack
        Result result;          // the result
    } AckReply;

    Stream& operator <<(Stream& os, const AckReply& reply);
    Stream& operator >>(Stream& is, AckReply& reply);

}

#endif // __ROLAND_SYSEX_H__