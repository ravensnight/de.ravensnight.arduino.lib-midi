#ifndef __MidiQueue_h__
#define __MidiQueue_h__

#include <Arduino.h>

#include <midi/MidiCommon.h>
#include <midi/MidiSink.h>
#include <midi/MidiReceiver.h>

#include <async/Mutex.h>
#include <async/Task.h>
#include <async/Queue.hpp>
#include <async/QueueListener.hpp>

using namespace ravensnight::async;

namespace ravensnight::midi {

    class MidiQueue : public MidiReceiver {

        private:

            Mutex _mutex;
            int8_t _cable;

            Queue<MidiEvent> *_queue = 0;
            Task _clientTask;
            MidiSink* _sink = 0;
            QueueListener<MidiEvent>* _listener = 0;

        public:

            /**
             * Create a midi queue that receives events only for the given cable
             * @param name the name of the queue, this name will be taken for client task and mutex
             * @param cable the cable to accept messages for, put -1 to receive all
             * @param qLength the length of the queue
             * @param qWaitTimeMS the time to wait for pushing a message to queue, if full.
             * @param taskPriority the priority of the listener/client task.
             * @param taskStackSize the stack size required by the receiver being set.
             */
            MidiQueue(const char* name, uint8_t cable, size_t qLength, uint32_t qWaitTimeMS, uint8_t taskPriority, uint32_t taskStackSize);

            /**
             * Cleanup
             */
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
