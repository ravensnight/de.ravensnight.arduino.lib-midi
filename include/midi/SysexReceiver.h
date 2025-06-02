#ifndef __SYSEX_RECEIVER_H__
#define __SYSEX_RECEIVER_H__

#include <midi/MidiReceiver.h>
#include <midi/SysexHandler.h>

#include <async/Mutex.h>

using namespace ravensnight::utils;
using namespace ravensnight::async;

namespace ravensnight::midi {

    enum class SysexState : uint8_t {
        WAITING = 0,
        READING = 1
    };

    class SysexReceiver : public MidiReceiver {

        private:

            SysexState _state = SysexState::WAITING;            
            Mutex _mutex;

            Buffer _buffer;
            size_t _msgLen;

            SysexHandler* _handler;

            bool unsafeAppend(const MidiEvent& evt);
            void unsafeReset();

        public:

            SysexReceiver(size_t bufferSize, SysexHandler* handler);
            ~SysexReceiver();

            void handle(const MidiEvent& event);
            void reset();
    };

}

#endif // __SYSEX_RECEIVER_H__
