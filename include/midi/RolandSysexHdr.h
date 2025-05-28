#ifndef __ROLAND_SYSEX_HDR_H__
#define __ROLAND_SYSEX_HDR_H__

#include <Arduino.h>
#include <StreamOperators.h>
#include <midi/RolandSysexAddr.h>

namespace MIDI {

    class RolandSysexHdr {
        public:

        uint8_t device;
        uint8_t model;
        uint8_t cmd;             
        RolandSysexAddr addr;

        friend Stream& operator >>(Stream& is, RolandSysexHdr& hdr);
        friend Stream& operator <<(Stream& os, const RolandSysexHdr& hdr);
    };
}


#endif // __ROLAND_SYSEX_HDR_H__