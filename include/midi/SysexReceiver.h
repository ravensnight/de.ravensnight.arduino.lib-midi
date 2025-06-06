#ifndef __SYSEX_RECEIVER_H__
#define __SYSEX_RECEIVER_H__

#include <midi/MidiReceiver.h>
#include <midi/SysexHandler.h>

#include <async/Mutex.h>

using namespace ravensnight::utils;
using namespace ravensnight::async;

namespace ravensnight::midi {

    enum class SysexState : uint8_t {
        open = 0,
        reading = 1,
        ignore = 2        
    };

    class SysexReceiver : public MidiReceiver {

        private:

            Mutex           _mutex;
            Buffer          _buffers[2];
            uint8_t         _activeBuffer;
            SysexHandler*   _handler;
            SysexState      _state;

            bool accepted(CINType type);

            bool unsafeAppend(const uint8_t* buffer, uint8_t len);
            void unsafeReset();
            void unsafeTrigger();

        public:

            SysexReceiver(size_t bufferSize, SysexHandler* handler);
            ~SysexReceiver();

            void handle(const MidiEvent& event);
    };

}

#endif // __SYSEX_RECEIVER_H__
