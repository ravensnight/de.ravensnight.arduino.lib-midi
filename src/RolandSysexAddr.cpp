#include <midi/LoggerConfig.h>
#include <midi/RolandSysexAddr.h>

using namespace ravensnight::logging;

namespace ravensnight::midi {

    Stream& operator << (Stream& os, const RolandSysexAddr& addr) {
        os << addr.get7bitHSB();
        os << addr.get7bitMSB();
        os << addr.get7bitLSB();

        return os;
    }

    Stream& operator >> (Stream& is, RolandSysexAddr& addr) {

        uint8_t value[3] = {0};
        uint32_t val;
        is.readBytes(value, 3);

        val = ((value[0] & 0x7F) << 14) | ((value[1] & 0x7F) << 7) | (value[2] & 0x7F);
        addr.set(val);

        return is;
    }

    RolandSysexChecksum& operator<<(RolandSysexChecksum& chksum, const RolandSysexAddr& addr) {

        RolandSysexAddr::_logger.trace("Add address to checksum: %06x", addr.get());

        chksum.add(addr.get7bitHSB());
        chksum.add(addr.get7bitMSB());
        chksum.add(addr.get7bitLSB());

        return chksum;
    }

    RolandSysexAddr::RolandSysexAddr() {
        this->_addr = 0;
    }

    RolandSysexAddr::RolandSysexAddr(const uint32_t addr) {
        set(addr);
    }

    uint32_t RolandSysexAddr::get() const {
        return _addr;
    }

    void RolandSysexAddr::set(const uint32_t val) {
        _addr = 0x1FFFFF & val;
    }

    uint8_t RolandSysexAddr::get7bitHSB() const {
        return (uint8_t)((_addr >> 14) & 0x7F);
    }
    
    uint8_t RolandSysexAddr::get7bitMSB() const {
        return (uint8_t)((_addr >> 7) & 0x7F);
    }

    uint8_t RolandSysexAddr::get7bitLSB() const {
        return (uint8_t)(_addr & 0x7F);
    }

    ClassLogger RolandSysexAddr::_logger(LC_MIDI_COMMON);
}