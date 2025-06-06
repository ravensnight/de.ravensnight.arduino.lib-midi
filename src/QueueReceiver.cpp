#include <midi/QueueReceiver.h>

using namespace ravensnight::async;

namespace ravensnight::midi {

    QueueReceiver::QueueReceiver(MidiReceiver& receiver) {
        _receiver = &receiver;
    }

    void QueueReceiver::handle(const MidiEvent& evt) {
        _receiver->handle(evt);
    }
}