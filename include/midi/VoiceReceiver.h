#ifndef __VOICE_RECEIVER_H__
#define __VOICE_RECEIVER_H__

#include <midi/MidiReceiver.h>
#include <midi/VoiceCallback.h>

#include <async/Mutex.h>

using namespace ravensnight::async;
namespace ravensnight::midi {

    class VoiceReceiver : public MidiReceiver {

        private:

            Mutex _mutex;
            VoiceCallback* _cb;
            bool accepted(CINType type);

        public:

            VoiceReceiver(VoiceCallback* cb);
            ~VoiceReceiver();

            void handle(const MidiEvent& event);
    };

}


#endif // __VOICE_RECEIVER_H__