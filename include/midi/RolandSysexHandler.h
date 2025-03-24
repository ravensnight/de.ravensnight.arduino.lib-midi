#ifndef __ROLAND_SYSEX_HANDLER_H__
#define __ROLAND_SYSEX_HANDLER_H__

#include <midi/MidiReader.h>
#include <midi/MidiWriter.h>
#include <midi/SysexHandler.h>
#include <midi/RolandSysexCallback.h>

namespace MIDI {

    class RolandSysexHandler : public SysexHandler {

        public:

            RolandSysexHandler(RolandSysexCallback* cb, MidiWriter* writer);

            // implements function from SysexHandler
            void onSysEx(uint8_t manufacturer, MidiReader* reader);

        private:

            MidiWriter* _writer = 0;
            RolandSysexCallback* _callback = 0;

            static void checksumAdd(int& previous, uint8_t value);

            /**
             * Parse a message from Midi In-Stream and potentially respond to writer.
             * Returns the number of bytes read or sent as payload or -1, if address was invalid or message could not be parsed correctly.
             */
            static int readSysEx(MidiReader* reader, MidiWriter* writer, RolandSysexCallback* cb);

            /**
             * Calculate the checksum
             */
            static uint8_t checksum(RolandAddr& addr, uint8_t bytes[], uint16_t len);

    };
}

#endif // __ROLAND_SYSEX_HANDLER_H__