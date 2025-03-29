#ifndef __SYSEX_RECEIVER_H__
#define __SYSEX_RECEIVER_H__

#include <midi/MidiReceiver.h>
#include <midi/SysexHandler.h>

namespace MIDI {

    enum class SysexState : uint8_t {
        WAITING = 0,
        READING = 1
    };

    class SysexReceiver : public MidiReceiver {

        private:

            SysexState _state = SysexState::WAITING;
            uint8_t* _buffer = 0;
            size_t _bufferLen = 0;
            size_t _bufferPos = 0;

            SysexHandler* _handler;

            bool append(const uint8_t* msg, size_t len);

        public:

            SysexReceiver(size_t bufferSize, SysexHandler* handler);
            ~SysexReceiver();

            void handle(CINType type, const uint8_t* msg, size_t len);
            void reset();
    };

}

#endif // __SYSEX_RECEIVER_H__
