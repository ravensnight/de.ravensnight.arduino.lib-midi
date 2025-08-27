#ifndef __MidiQueue_h__
#define __MidiQueue_h__

#include <Arduino.h>

#include <midi/MidiCommon.h>
#include <midi/MidiSink.h>
#include <midi/MidiReceiver.h>

#include <async/Mutex.h>
#include <async/Queue.hpp>
#include <async/QueueListener.hpp>

using namespace ravensnight::async;

namespace ravensnight::midi {

    class MidiQueue : public MidiReceiver {

        private:

            Mutex _mutex;
            int8_t _cable;

            Queue<MidiEvent> *_queue = 0;
            MidiSink* _sink = 0;
            QueueListener<MidiEvent>* _listener = 0;

        public:

            /**
             * Create a midi queue that receives events for all cables
             */
            MidiQueue(size_t qLength, uint32_t qWaitTimeMS);

            /**
             * Create a midi queue that receives events only for the given cable
             */
            MidiQueue(uint8_t cable, size_t qLength, uint32_t qWaitTimeMS);
            ~MidiQueue();

            /**
             * Bind the receiver that will get the midi messages asynchronously.
             */
            void set(MidiReceiver* receiver);

            /**
             * Put a midi event to the queue.
             */
            void handle(const MidiEvent& event);

            /**
             * Setup and install the queue.
             */
            bool install();

    };

}


#endif
