#include <midi/MidiSource.h>
#include <async/LockGuard.h>

namespace ravensnight::midi {

    MidiSource::MidiSource(Queue<MidiEvent>& q) {
        _queue = &q;
        _cable = -1;
    }

    MidiSource::MidiSource(uint8_t cable, Queue<MidiEvent>& q) {
        _queue = &q;
        _cable = cable & 0xFF;
    }

    void MidiSource::handle(const MidiEvent& evt) {
        if ((evt.cable == _cable) || _cable < 0) {
            _queue->push(evt);
        }
    }
}
