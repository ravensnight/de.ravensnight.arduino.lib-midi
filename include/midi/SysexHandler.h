#ifndef __SYSEX_HANDLER_H__
#define __SYSEX_HANDLER_H__

#include <Arduino.h>

namespace MIDI {

    // predeclare MidiReader*
    class MidiReader;

    // Sysex Handler declaration
    class SysexHandler {

        public:
            virtual void onSysEx(uint8_t manufacturer, MidiReader* reader) = 0;

    };

}

#endif // __SYSEX_HANDLER_H__