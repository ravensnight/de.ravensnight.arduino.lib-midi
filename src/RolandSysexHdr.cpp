#include <midi/RolandSysexHdr.h>
#include <Logger.h>

using namespace MIDI;
using namespace LOGGING;

ByteInputStream& operator >>(ByteInputStream& is, __RolandSysexHdr& hdr) {
    is >> hdr.device;
    is >> hdr.model;
    is >> hdr.cmd;
    is.readBytes(hdr.addr, 3);

    return is;
}

ByteOutputStream& operator <<(ByteOutputStream& os, __RolandSysexHdr& hdr) {
    os << hdr.device;
    os << hdr.model;
    os << hdr.cmd;
    os.writeBytes(hdr.addr, 3);

    return os;
}
