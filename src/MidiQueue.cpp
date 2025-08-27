#include <midi/MidiQueue.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;

MidiQueue::MidiQueue(const char* name, size_t qLength, uint32_t qWaitTimeMS) : Service(name) {
    _queue = new Queue<MidiEvent>(qLength, false, qWaitTimeMS);
}

bool MidiQueue::preInstall() {    

    if (_sink == 0) {
        Logger::warn("No Receiver had been set. Cannot (re)install.");
        return false;
    }

    return this->_queue->install();
}

void MidiQueue::postUninstall() {    
    if (_sink != 0) {
        delete _sink;
        _sink = 0;
    }
 }

void MidiQueue::set(MidiReceiver* receiver) {
    this->set(receiver, MIDITASK_DEFAULT_PRIORITY, MIDITASK_DEFAULT_STACKSIZE);
}

void MidiQueue::set(MidiReceiver* receiver, uint8_t taskPriority, uint32_t stackSize) {
    if (receiver == 0) {
        Logger::warn("Receiver must not be NULL");
        return;
    }

    if (_sink != 0) {        
        delete _sink;
    }

    _sink = new MidiSink(receiver);
    _taskPriority = taskPriority;
    _taskStackSize = stackSize;
}

/**
 * Send one event to queue
 */
void MidiQueue::handle(const MidiEvent& evt) {
    _queue->push(evt);
}

uint8_t MidiQueue::getPriority() {
    return this->_taskPriority;
}

uint32_t MidiQueue::getStackSize() {
    return this->_taskStackSize;
}

Runnable* MidiQueue::createRunnable() {
    if (_sink != 0) {
        return new QueueListener<MidiEvent>(_queue, _sink, false);
    }

    return 0;
}
