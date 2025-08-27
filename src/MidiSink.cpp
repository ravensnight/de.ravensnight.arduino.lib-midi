#include <midi/MidiSink.h>

using namespace ravensnight::async;

namespace ravensnight::midi {

    MidiSink::MidiSink(MidiReceiver* receiver) {
        _receiver = receiver;
    }

    void MidiSink::handle(const MidiEvent& evt) {
        _receiver->handle(evt);
    }
}
