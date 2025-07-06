#include <midi/RolandSysexTypes.h>

namespace ravensnight::midi {

    Stream& operator <<(Stream& os, const AckReply& reply) {
        os << reply.addr;
        os << (uint8_t)reply.result;
        return os;
    }

    Stream& operator >>(Stream& is, AckReply& reply) {
        uint8_t tmp;
        is >> reply.addr;
        is >> tmp; reply.result = (Result)tmp;
        return is;
    }

}