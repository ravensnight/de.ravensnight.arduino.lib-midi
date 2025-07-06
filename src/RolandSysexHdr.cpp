#include <midi/RolandSysexHdr.h>
#include <midi/RolandSysexAddr.h>
#include <StreamHelper.h>

using namespace ravensnight::utils;
namespace ravensnight::midi {

    Stream& operator >>(Stream& is, RolandSysexHdr& hdr) {

        is >> hdr.device;
        is >> hdr.model;
        is >> hdr.cmd;
        is >> hdr.addr;

        return is;
    }

    Stream& operator <<(Stream& os, RolandSysexHdr& hdr) {
        os << hdr.device;
        os << hdr.model;
        os << hdr.cmd;
        os << hdr.addr;

        return os;
    }

}