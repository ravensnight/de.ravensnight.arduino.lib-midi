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
            RolandSysexAddr(const uint32_t addr);

            /**
             * Create a roland addr from 7bit coded 3bytes
             */
            RolandSysexAddr(const uint8_t value[3]);

            /**
             * Provide the address value.
             */
            uint32_t get() const; 

            /**
             * Read as 7bit value
             */
            void read(uint8_t value[3]) const;

            /**
             * Set from 8bit coded
             */
            void set(const uint32_t val);

            /**
             * Set from 7bit coded
             */
            void write(const uint8_t value[3]);

            uint8_t get7bitHSB() const;
            uint8_t get7bitMSB() const;
            uint8_t get7bitLSB() const;

        private:

            uint32_t  addr;

    };

}


#endif // __ROLAND_SYSEX_ADDR_H__