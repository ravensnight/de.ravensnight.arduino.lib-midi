#include <midi/RolandSysexChecksum.h>

namespace MIDI {

    RolandSysexChecksum::RolandSysexChecksum() {
        _value = 0;
    }

    void RolandSysexChecksum::reset() {
        _value = 0;
    }

    uint8_t RolandSysexChecksum::value() const {
        int res = 128 - _value;
        if (res == 128) res = 0;

        return res;
    }

    void RolandSysexChecksum::add(uint8_t val) {
        _value += val;
        if (_value > 127) {
            _value -= 128;
        }
    }

    RolandSysexChecksum& operator <<(RolandSysexChecksum& chksum, uint8_t value) {
        chksum.add(value);
        return chksum;
    }

    RolandSysexChecksum& operator <<(RolandSysexChecksum& chksum, Buffer& buffer) {
        for (size_t i = 0; i < buffer.length(); i++) {
            chksum.add(buffer[i]);
        }
        return chksum;
    }

    Stream& operator<<(Stream& os, const RolandSysexChecksum& chksum) {
        os << chksum.value();
        return os;
    }

}
