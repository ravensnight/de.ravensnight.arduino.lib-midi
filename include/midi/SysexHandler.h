#ifndef __SYSEX_HANDLER_H__
#define __SYSEX_HANDLER_H__

#include <Arduino.h>
#include <midi/ArrayInputStream.h>

namespace MIDI {

    // Sysex Handler declaration
    class SysexHandler {

        public:
            virtual void onSysEx(Stream* inputStream) = 0;

    };

}

#endif // __SYSEX_HANDLER_H__