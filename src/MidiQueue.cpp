#include <midi/MidiQueue.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;

MidiQueue::MidiQueue(const char* name, size_t qLength, uint32_t qWaitTimeMS) : 
    _mutex(name),
    _clientTask(name)
{
    _queue = new Queue<MidiEvent>(qLength, false, qWaitTimeMS);
}

MidiQueue::~MidiQueue() {
    if (_listener != 0) {
        _clientTask.kill();
        delete _listener;
    }

    if (_sink != 0) {
        delete _sink;
    }
}

bool MidiQueue::install() {    
    acquirelock(_mutex);

    if (_sink == 0) {
        Logger::warn("No Receiver had been set. Cannot (re)install.");
        return false;
    }

    if (_listener != 0) {
        _clientTask.kill();
        delete _listener;
    }

    this->_queue->install();

    _listener = new QueueListener<MidiEvent>(_queue, _sink, false);
    _clientTask.start(_listener, _taskPriority, _taskStackSize);

    return true;
}

void MidiQueue::set(MidiReceiver* receiver) {
    this->set(receiver, MIDITASK_DEFAULT_PRIORITY, MIDITASK_DEFAULT_STACKSIZE);
}

void MidiQueue::set(MidiReceiver* receiver, uint8_t taskPriority, uint32_t stackSize) {
    if (receiver == 0) {
        Logger::warn("Receiver must not be NULL");
        return;
    }

    acquirelock(_mutex);

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