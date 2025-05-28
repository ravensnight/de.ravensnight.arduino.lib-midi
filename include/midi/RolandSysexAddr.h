#ifndef __ROLAND_SYSEX_ADDR_H__
#define __ROLAND_SYSEX_ADDR_H__

#include <Arduino.h>
#include <midi/RolandSysexChecksum.h>
namespace MIDI {

    class RolandSysexAddr {
        private:

            uint32_t  _addr;

        public:

            RolandSysexAddr();

            /**
             * Create a roland addr from given 21bit value.
             */
            RolandSysexAddr(const uint32_t addr);

            /**
             * Provide the address value.
             */
            uint32_t get() const; 

            /**
             * Set from 8bit coded
             */
            void set(const uint32_t val);

            uint8_t get7bitHSB() const;
            uint8_t get7bitMSB() const;
            uint8_t get7bitLSB() const;

            friend Stream& operator >> (Stream& is, RolandSysexAddr& addr);
            friend Stream& operator << (Stream& os, const RolandSysexAddr& addr);
            friend RolandSysexChecksum& operator<<(RolandSysexChecksum& chksum, const RolandSysexAddr& addr);

    };

}


#endif // __ROLAND_SYSEX_ADDR_H__