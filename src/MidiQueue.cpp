#include <midi/MidiQueue.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;

MidiQueue::MidiQueue(size_t qLength, uint32_t qWaitTimeMS) : _mutex("MidiQueue") {
    _queue = new Queue<MidiEvent>(qLength, false, qWaitTimeMS);
    _cable = -1;
}

MidiQueue::MidiQueue(uint8_t cable, size_t qLength, uint32_t qWaitTimeMS) : _mutex("MidiQueue") {
    _queue = new Queue<MidiEvent>(qLength, false, qWaitTimeMS);
    _cable = cable & 0xFF;
}

MidiQueue::~MidiQueue() {
    if (_listener != 0) {
        delete _listener;
    }

    if (_sink != 0) {
        delete _sink;
    }
}

bool MidiQueue::install() {
    return this->_queue->install();
}

void MidiQueue::set(MidiReceiver* receiver) {
    acquirelock(_mutex);

    if (_listener != 0) {
        delete _listener;
    }

    if (_sink != 0) {
        delete _sink;
    }

    _sink = new MidiSink(receiver);
    _listener = new QueueListener<MidiEvent>(_queue, _sink, false);
}

/**
 * Send one event to queue
 */
void MidiQueue::handle(const MidiEvent& evt) {
    if ((evt.cable == _cable) || _cable < 0) {
        _queue->push(evt);
    }
}