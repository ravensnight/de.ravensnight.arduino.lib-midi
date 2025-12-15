#ifndef __QueueReceiver_h__
#define __QueueReceiver_h__

#include <Arduino.h>
#include <midi/MidiCommon.h>
#include <midi/MidiReceiver.h>

#include <async/Queue.hpp>
#include <utils/Ref.hpp>

using namespace ravensnight::utils;
using namespace ravensnight::async;

namespace ravensnight::midi {

    /**
     * Defines the Q-Receiver for midi events. This class is used by MidiQueue.
     */
    class MidiSink : public Receiver<MidiEvent> {
        private:
            Ref<MidiReceiver> _receiver;

        public:

            MidiSink(Ref<MidiReceiver>& receiver);
            void handle(const MidiEvent& evt);

    };

}


#endif // __QueueReceiver_h__