#ifndef __ROLAND_SYSEX_HANDLER_H__
#define __ROLAND_SYSEX_HANDLER_H__

#include <midi/MidiTransmitter.h>
#include <midi/SysexHandler.h>
#include <midi/RolandSysexCallback.h>
#include <Converter.h>
#include <mutex>

namespace ravensnight::midi {

    class RolandSysexHandler : public SysexHandler {

        public:

            RolandSysexHandler(RolandSysexCallback* cb, MidiTransmitter* out);
            ~RolandSysexHandler();

            // implements function from SysexHandler
            void onSysEx(const uint8_t* buffer, size_t len);

        private:

            std::mutex _mutex;
            MidiTransmitter* _out;
            RolandSysexCallback* _cb;
            Converter* _conv[2];

            /**
             * Parse a message from Midi In-Stream and potentially respond to writer.
             * Returns the number of bytes read or sent as payload or -1, if address was invalid or message could not be parsed correctly.
             */
            int handleSysEx(const uint8_t* buffer, size_t len);
            int handleCmdRead(RolandSysexAddr& addr, BufferInputStream& inputStream);
            int handleCmdWrite(RolandSysexAddr& addr, BufferInputStream& inputStream);

    };
}

#endif // __ROLAND_SYSEX_HANDLER_H__