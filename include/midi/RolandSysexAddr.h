#ifndef __ROLAND_SYSEX_ADDR_H__
#define __ROLAND_SYSEX_ADDR_H__

#include <Arduino.h>

namespace MIDI {

    class RolandSysexAddr {
        public:

            RolandSysexAddr();

            /**
             * Create a roland addr from given 21bit value.
             */
            RolandSysexAddr(uint32_t addr);

            /**
             * Create a roland addr from 7bit coded 3bytes
             */
            RolandSysexAddr(uint8_t hsb, uint8_t msb, uint8_t lsb);

            /**
             * Provide the address value.
             */
            uint32_t value(); 


            uint8_t get7BitHSB();
            uint8_t get7BitMSB();
            uint8_t get7BitLSB();

        private:

            uint32_t  addr;

    };

}


#endif // __ROLAND_SYSEX_ADDR_H__