#ifndef __ROLAND_SYSEX_HANDLER_H__
#define __ROLAND_SYSEX_HANDLER_H__

#include <midi/MidiTransmitter.h>
#include <midi/SysexHandler.h>
#include <midi/RolandSysexCallback.h>
#include <Converter.h>

#include <BufferInputStream.h>
#include <async/Mutex.h>

using namespace ravensnight::async;
using namespace ravensnight::utils;

namespace ravensnight::midi {

    enum class Stage : uint8_t {
        undefined = 0,
        manufacturer = 1,
        device = 2,
        model = 3,
        command = 4,
        address = 5,
        payload = 6,
        checksum = 7,
        complete = 8
    };
    class RolandSysexHandler : public SysexHandler {

        public:

            RolandSysexHandler(size_t bufferSize, RolandSysexCallback* cb, MidiTransmitter* out);
            ~RolandSysexHandler();

            void init();
            void append(uint8_t);
            void commit();
            bool ready();

        private:

            RolandSysexAddr     _reqAddress;
            RolandSysexChecksum _reqChecksum;
            AddressInfo         _reqAddressInfo;
            Buffer              _reqBuffer;
            Command             _reqCommand;
            size_t              _reqPayloadSize;
            Stage               _stage;

            MidiTransmitter* _out;
            RolandSysexCallback* _cb;
            Converter* _conv[2];

            void reset();

            /**
             * Parse a message from Midi In-Stream and potentially respond to writer.
             * Returns the number of bytes read or sent as payload or -1, if address was invalid or message could not be parsed correctly.
             */
            void handleCmdRead(RolandSysexAddr& addr, BufferInputStream& inputStream);
            void handleCmdWrite(RolandSysexAddr& addr, BufferInputStream& inputStream);

            /**
             * Send some reply message out to client.
             */
            void sendReply(RolandSysexAddr& addr, const uint8_t* decodedPayload, size_t len);

    };
}

#endif // __ROLAND_SYSEX_HANDLER_H__