#include <midi/MidiReceiver.h>

namespace MIDI {
    MidiReceiver& operator<<(MidiReceiver& q, const MidiEvent& event) {
        q.handle(event);
        return q;
    }
}
