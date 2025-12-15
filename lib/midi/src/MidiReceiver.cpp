#include <midi/MidiReceiver.h>

namespace ravensnight::midi {
    MidiReceiver& operator<<(MidiReceiver& q, const MidiEvent& event) {
        q.handle(event);
        return q;
    }
}
