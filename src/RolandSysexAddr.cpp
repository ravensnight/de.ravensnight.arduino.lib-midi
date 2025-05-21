#include <midi/RolandSysexAddr.h>

using namespace MIDI;

RolandSysexAddr::RolandSysexAddr() {
    this->addr = 0;
}

RolandSysexAddr::RolandSysexAddr(uint32_t addr) {
    this->addr = 0x1FFFFF & addr;
}

RolandSysexAddr::RolandSysexAddr(uint8_t hsb, uint8_t msb, uint8_t lsb) {
    this->addr = ((hsb & 0x7F) << 14) | ((msb & 0x7F) << 7) | (lsb & 0x7F);
}

uint32_t RolandSysexAddr::value() {
    return addr;
}

uint8_t RolandSysexAddr::get7BitHSB() {
    return ((addr >> 14) & 0x7F);
}

uint8_t RolandSysexAddr::get7BitMSB() {
    return ((addr >> 7) & 0x7F);
}

uint8_t RolandSysexAddr::get7BitLSB() {
    return (addr & 0x7F);
}