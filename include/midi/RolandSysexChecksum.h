#ifndef __RolandSysexChecksum_h__
#define __RolandSysexChecksum_h__

#include <Arduino.h>
#include <Buffer.h>

namespace MIDI {

    class RolandSysexChecksum {

        private:

            int _value;

        public:

            RolandSysexChecksum();

            void add(uint8_t value);
            void add(const uint8_t* buffer, size_t len);

            void reset();

            uint8_t value() const;

            friend RolandSysexChecksum& operator<<(RolandSysexChecksum& chksum, uint8_t value);
            friend RolandSysexChecksum& operator<<(RolandSysexChecksum& chksum, Buffer& buffer);

            friend Stream& operator<<(Stream& os, const RolandSysexChecksum& chksum);

    };
}

#endif // __RolandSysexChecksum_h__