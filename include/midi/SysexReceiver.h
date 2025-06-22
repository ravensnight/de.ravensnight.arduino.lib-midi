#ifndef __SYSEX_RECEIVER_H__
#define __SYSEX_RECEIVER_H__

#include <midi/MidiReceiver.h>
#include <midi/SysexHandler.h>
#include <async/WeakLock.h>

using namespace ravensnight::async;
namespace ravensnight::midi {
    class SysexReceiver : public MidiReceiver {

        private:

            WeakLock        _lock;
            SysexHandler*   _handler;

            bool accepted(CINType type);
            void unsafeAppend(const uint8_t* buffer, uint8_t len);

        public:

            SysexReceiver(SysexHandler* handler);
            ~SysexReceiver();

            bool ready();

            void handle(const MidiEvent& event);
    };

}

#endif // __SYSEX_RECEIVER_H__
