#ifndef __ROLAND_SYSEX_HDR_H__
#define __ROLAND_SYSEX_HDR_H__

#include <Arduino.h>
#include <ByteInputStream.h>
#include <ByteOutputStream.h>

namespace MIDI {

    typedef struct __RolandSysexHdr {
        uint8_t device;
        uint8_t model;
        uint8_t cmd;             
        uint8_t addr[3];

        friend ByteInputStream& operator >>(ByteInputStream& is, __RolandSysexHdr& hdr);
        friend ByteOutputStream& operator <<(ByteOutputStream& os, const __RolandSysexHdr& hdr);
        
    } RolandSysexHdr;


}


#endif // __ROLAND_SYSEX_HDR_H__