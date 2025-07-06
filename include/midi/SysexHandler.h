#ifndef __SYSEX_HANDLER_H__
#define __SYSEX_HANDLER_H__

#include <Arduino.h>
namespace ravensnight::midi {

    // Sysex Handler declaration
    class SysexHandler {

        public:

            virtual void init() = 0;

            virtual void append(uint8_t) = 0;

            virtual void commit() = 0;

            virtual bool ready() = 0;

    };

}

#endif // __SYSEX_HANDLER_H__