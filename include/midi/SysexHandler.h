#ifndef __SYSEX_HANDLER_H__
#define __SYSEX_HANDLER_H__

#include <Arduino.h>
#include <BufferInputStream.h>

namespace MIDI {

    // Sysex Handler declaration
    class SysexHandler {

        public:
            virtual void onSysEx(const uint8_t* message, size_t len) = 0;

    };

}

#endif // __SYSEX_HANDLER_H__