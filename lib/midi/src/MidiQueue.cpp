#include <midi/LoggerConfig.h>
#include <midi/MidiQueue.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;

MidiQueue::MidiQueue(const char* name, size_t qLength, uint32_t qWaitTimeMS) : 
    Service(name),
    _queue(RefType::owned),
    _sink(RefType::owned)
{
    _queue = new Queue<MidiEvent>(qLength, false, qWaitTimeMS);
}

bool MidiQueue::preInstall() {
    if (_sink.isNULL()) {
        _logger.warn("No Receiver had been set. Cannot (re)install.");
        return false;
    }

    return (*_queue).install();
}

void MidiQueue::postUninstall() {    
    _sink.clear();
}

void MidiQueue::set(Ref<MidiReceiver>& receiver) {
    this->set(receiver, MIDITASK_DEFAULT_PRIORITY, MIDITASK_DEFAULT_STACKSIZE);
}

void MidiQueue::set(Ref<MidiReceiver>& receiver, uint8_t taskPriority, uint32_t stackSize) {
    if (receiver.isNULL()) {
        _logger.warn("Receiver must not be NULL");
        return;
    }

    _sink = new MidiSink(receiver);
    _taskPriority = taskPriority;
    _taskStackSize = stackSize;
}

/**
 * Send one event to queue
 */
void MidiQueue::handle(const MidiEvent& evt) {
    (*_queue).push(evt);
}

uint8_t MidiQueue::getPriority() {
    return this->_taskPriority;
}

uint32_t MidiQueue::getStackSize() {
    return this->_taskStackSize;
}

Runnable* MidiQueue::createRunnable() {
    if (!_sink.isNULL()) {
        return new QueueListener<MidiEvent>(_queue.get(), _sink.get(), false);
    }

    return 0;
}

ClassLogger MidiQueue::_logger(LC_MIDI_COMMON);