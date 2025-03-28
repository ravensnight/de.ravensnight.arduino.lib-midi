#ifndef __ROLAND_SYSEX_HANDLER_H__
#define __ROLAND_SYSEX_HANDLER_H__

#include <midi/MidiTransmitter.h>
#include <midi/SysexHandler.h>
#include <midi/RolandSysexCallback.h>

namespace MIDI {

    class RolandSysexHandler : public SysexHandler {

        public:

            RolandSysexHandler(RolandSysexCallback* cb, MidiTransmitter* out);

            // implements function from SysexHandler
            void onSysEx(Stream* inputStream);

        private:

            MidiTransmitter* _out = 0;
            RolandSysexCallback* _cb = 0;

            static void checksumAdd(int& previous, uint8_t value);

            /**
             * Parse a message from Midi In-Stream and potentially respond to writer.
             * Returns the number of bytes read or sent as payload or -1, if address was invalid or message could not be parsed correctly.
             */
            static int readSysEx(Stream* inputStream, MidiTransmitter* out, RolandSysexCallback* cb);

            /**
             * Calculate the checksum
             */
            static uint8_t checksum(RolandAddr& addr, uint8_t bytes[], uint16_t len);

    };
}

#endif // __ROLAND_SYSEX_HANDLER_H__