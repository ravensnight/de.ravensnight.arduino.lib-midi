#include <midi/RolandSysexHdr.h>
#include <Logger.h>

using namespace MIDI;
using namespace LOGGING;

Stream& operator >>(Stream& is, __RolandSysexHdr& hdr) {
    is >> hdr.device;
    is >> hdr.model;
    is >> hdr.cmd;
    is.readBytes(hdr.addr, 3);

    return is;
}

Stream& operator <<(Stream& os, __RolandSysexHdr& hdr) {
    os << hdr.device;
    os << hdr.model;
    os << hdr.cmd;
    os << hdr.addr[0];
    os << hdr.addr[1];
    os << hdr.addr[2];

    return os;
}
