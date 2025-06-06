#include <midi/QueueSender.h>

namespace ravensnight::midi {

    QueueSender::QueueSender(Queue<MidiEvent>& q) {
        _queue = &q;
        _cable = -1;
    }

    QueueSender::QueueSender(uint8_t cable, Queue<MidiEvent>& q) {
        _queue = &q;
        _cable = cable & 0xFF;
    }

    void QueueSender::handle(const MidiEvent& evt) {
        if ((evt.cable == _cable) || _cable < 0) {
            _queue->push(evt);
        }
    }

}
