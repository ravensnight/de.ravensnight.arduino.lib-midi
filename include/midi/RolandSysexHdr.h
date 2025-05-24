#ifndef __ROLAND_SYSEX_HDR_H__
#define __ROLAND_SYSEX_HDR_H__

#include <Arduino.h>
#include <StreamOperators.h>

namespace MIDI {

    typedef struct __RolandSysexHdr {
        uint8_t device;
        uint8_t model;
        uint8_t cmd;             
        uint8_t addr[3];

        friend Stream& operator >>(Stream& is, __RolandSysexHdr& hdr);
        friend Stream& operator <<(Stream& os, const __RolandSysexHdr& hdr);

    } RolandSysexHdr;


}


#endif // __ROLAND_SYSEX_HDR_H__