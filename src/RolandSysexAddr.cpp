#include <midi/RolandSysexAddr.h>

using namespace MIDI;

RolandSysexAddr::RolandSysexAddr() {
    this->addr = 0;
}

RolandSysexAddr::RolandSysexAddr(const uint32_t addr) {
    set(addr);
}

RolandSysexAddr::RolandSysexAddr(const uint8_t addr[3]) {
    write(addr);
}

uint32_t RolandSysexAddr::get() const {
    return addr;
}

void RolandSysexAddr::set(const uint32_t val) {
    addr = 0x1FFFFF & val;
}

void RolandSysexAddr::write(const uint8_t value[3]) {
    addr = ((value[0] & 0x7F) << 14) | ((value[1] & 0x7F) << 7) | (value[2] & 0x7F);
}

void RolandSysexAddr::read(uint8_t value[3]) const {
    value[0] = get7bitHSB();
    value[1] = get7bitMSB();
    value[2] = get7bitLSB();
}

uint8_t RolandSysexAddr::get7bitHSB() const {
    return ((addr >> 14) & 0x7F);
}

uint8_t RolandSysexAddr::get7bitMSB() const {
    return ((addr >> 7) & 0x7F);
}

uint8_t RolandSysexAddr::get7bitLSB() const {
    return (addr & 0x7F);
}