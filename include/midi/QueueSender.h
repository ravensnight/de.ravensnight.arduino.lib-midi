#ifndef __QueuingEventReceiver_h__
#define  __QueuingEventReceiver_h__

#include <midi/MidiCommon.h>
#include <midi/MidiDevice.h>
#include <async/Queue.hpp>

using namespace ravensnight::midi;
using namespace ravensnight::async;

namespace ravensnight::midi {

    class QueueSender : public MidiReceiver {

        private:
            Queue<MidiEvent>* _queue;
            int _cable;

        public:

            QueueSender(Queue<MidiEvent>& queue);
            QueueSender(uint8_t cable, Queue<MidiEvent>& queue);

            void handle(const MidiEvent& event);
    };

}


#endif //  __QueuingEventReceiver_h__