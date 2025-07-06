#include <midi/QueueSender.h>
#include <async/LockGuard.h>

namespace ravensnight::midi {

    QueueSender::QueueSender(Queue<MidiEvent>& q) : _lock("QueueSender") {
        _queue = &q;
        _cable = -1;
    }

    QueueSender::QueueSender(uint8_t cable, Queue<MidiEvent>& q) : _lock("QueueSender") {
        _queue = &q;
        _cable = cable & 0xFF;
    }

    bool QueueSender::ready() {
        return !_lock.isLocked();
    }

    void QueueSender::handle(const MidiEvent& evt) {
        acquirelock(_lock);

        if ((evt.cable == _cable) || _cable < 0) {
            _queue->push(evt);
        }
    }

}
