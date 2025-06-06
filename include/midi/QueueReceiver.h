#ifndef __QueueReceiver_h__
#define __QueueReceiver_h__

#include <Arduino.h>
#include <midi/MidiCommon.h>
#include <midi/MidiReceiver.h>
#include <async/Queue.hpp>

using namespace ravensnight::async;

namespace ravensnight::midi {

    class QueueReceiver : public Receiver<MidiEvent> {
        private:
            MidiReceiver* _receiver;

        public:

            QueueReceiver(MidiReceiver& receiver);
            void handle(const MidiEvent& evt);

    };

}


#endif // __QueueReceiver_h__