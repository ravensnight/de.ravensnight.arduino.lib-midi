#ifndef __SYSEX_RECEIVER_H__
#define __SYSEX_RECEIVER_H__

#include <midi/MidiReceiver.h>
#include <midi/SysexHandler.h>
#include <mutex>

namespace MIDI {

    enum class SysexState : uint8_t {
        WAITING = 0,
        READING = 1
    };

    class SysexReceiver : public MidiReceiver {

        private:

            SysexState _state = SysexState::WAITING;
            
            std::mutex _mutex;

            Buffer _buffer;
            size_t _msgLen;

            SysexHandler* _handler;


            bool append(const MidiEvent& evt);

        public:

            SysexReceiver(size_t bufferSize, SysexHandler* handler);
            ~SysexReceiver();

            void handle(const MidiEvent& event);
            void reset();
    };

}

#endif // __SYSEX_RECEIVER_H__
