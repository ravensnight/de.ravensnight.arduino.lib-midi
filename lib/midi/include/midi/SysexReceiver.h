#ifndef __SYSEX_RECEIVER_H__
#define __SYSEX_RECEIVER_H__

#include <ClassLogger.h>
#include <midi/MidiReceiver.h>
#include <midi/SysexHandler.h>
#include <async/Mutex.h>
#include <utils/Ref.hpp>

using namespace ravensnight::utils;
using namespace ravensnight::async;

namespace ravensnight::midi {
    class SysexReceiver : public MidiReceiver {

        private:

            static ClassLogger _logger;
            
            Mutex _mutex;
            Ref<SysexHandler>  _handler;

            bool accepted(CINType type);
            void unsafeAppend(const uint8_t* buffer, uint8_t len);

        public:

            SysexReceiver(Ref<SysexHandler>& handler);
            ~SysexReceiver();

            void handle(const MidiEvent& event);
    };

}

#endif // __SYSEX_RECEIVER_H__
