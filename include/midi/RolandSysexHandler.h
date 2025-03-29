#ifndef __ROLAND_SYSEX_HANDLER_H__
#define __ROLAND_SYSEX_HANDLER_H__

#include <midi/MidiTransmitter.h>
#include <midi/SysexHandler.h>
#include <midi/RolandSysexCallback.h>

namespace MIDI {

    class RolandSysexHandler : public SysexHandler {

        public:

            RolandSysexHandler(RolandSysexCallback* cb, MidiTransmitter* out);
            ~RolandSysexHandler();

            // implements function from SysexHandler
            void onSysEx(ByteInputStream* inputStream);

        private:

            MidiTransmitter* _out;
            RolandSysexCallback* _cb;

            /**
             * Parse a message from Midi In-Stream and potentially respond to writer.
             * Returns the number of bytes read or sent as payload or -1, if address was invalid or message could not be parsed correctly.
             */
            int handleSysEx(ByteInputStream* inputStream);
            int handleCmdRead(RolandSysexHdr& hdr, ByteInputStream* inputStream);
            int handleCmdWrite(RolandSysexHdr& hdr, ByteInputStream* inputStream);

            /**
             * Calculate the checksum
             */
            static uint8_t checksum(RolandAddr& addr, uint8_t* bytes, uint16_t len);
            static void checksumAdd(int& previous, uint8_t value);

    };
}

#endif // __ROLAND_SYSEX_HANDLER_H__