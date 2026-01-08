#ifndef __VOICE_RECEIVER_H__
#define __VOICE_RECEIVER_H__

#include <Logger.h>

#include <midi/MidiReceiver.h>
#include <midi/VoiceCallback.h>

#include <async/Mutex.h>
#include <utils/Ref.hpp>

using namespace ravensnight::logging;
using namespace ravensnight::async;
using namespace ravensnight::utils;

namespace ravensnight::midi {

    class VoiceReceiver : public MidiReceiver {

        private:

            static Logger _logger;
            Mutex _mutex;
            Ref<VoiceCallback> _cb;
            bool accepted(CINType type);

        public:

            VoiceReceiver(Ref<VoiceCallback>& cb);
            ~VoiceReceiver();

            void handle(const MidiEvent& event);
    };

}


#endif // __VOICE_RECEIVER_H__