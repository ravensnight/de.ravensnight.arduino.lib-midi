#ifndef __MidiQueue_h__
#define __MidiQueue_h__

#include <Arduino.h>

#include <ClassLogger.h>

#include <midi/MidiCommon.h>
#include <midi/MidiSink.h>
#include <midi/MidiReceiver.h>

#include <async/Mutex.h>
#include <async/Service.h>
#include <async/Queue.hpp>
#include <async/QueueListener.hpp>

using namespace ravensnight::async;

#define MIDITASK_DEFAULT_PRIORITY 3
#define MIDITASK_DEFAULT_STACKSIZE 2048

namespace ravensnight::midi {

    class MidiQueue : public MidiReceiver, public Service {

        private:

            static ClassLogger _logger;

            uint8_t _taskPriority = MIDITASK_DEFAULT_PRIORITY;
            uint32_t _taskStackSize = MIDITASK_DEFAULT_STACKSIZE;

            Queue<MidiEvent> *_queue = 0;
            MidiSink* _sink = 0;

        protected:

            uint32_t getStackSize();
            uint8_t  getPriority();       
            Runnable* createRunnable();

            bool preInstall();
            void postUninstall();

        public:

            /**
             * Create a midi queue that receives events only for the given cable
             * @param name the name of the queue, this name will be taken for client task and mutex
             * @param qLength the length of the queue
             * @param qWaitTimeMS the time to wait for pushing a message to queue, if full.
             */
            MidiQueue(const char* name, size_t qLength, uint32_t qWaitTimeMS);

            /**
             * Set a receiver and use default task settings.
             * @see #set( MidiReceiver*, uint8_t, uint32_t )
             */
            void set(MidiReceiver* receiver);

            /**
             * Bind the receiver that will get the midi messages asynchronously.
             * @param receiver the listener
             * @param taskPriority the priority of the listener/client task.
             * @param taskStackSize the stack size required by the receiver being set.
             */
            void set(MidiReceiver* receiver, uint8_t taskPriority, uint32_t stackSize);

            /**
             * Put a midi event to the queue.
             */
            void handle(const MidiEvent& event);

    };

}


#endif
