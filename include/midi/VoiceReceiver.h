#ifndef __VOICE_RECEIVER_H__
#define __VOICE_RECEIVER_H__

#include <midi/MidiReceiver.h>
#include <midi/VoiceCallback.h>

namespace ravensnight::midi {

    class VoiceReceiver : public MidiReceiver {

        private:
            VoiceCallback* _cb;

            // check received CINType for acceptance
            bool accepted(CINType type);

        public:

            VoiceReceiver(VoiceCallback* cb);
            ~VoiceReceiver();

            void handle(const MidiEvent& event);
    };

}


#endif // __VOICE_RECEIVER_H__