#ifndef __MIDI_RECEIVER_H__
#define __MIDI_RECEIVER_H__

#include <Arduino.h>
#include <midi/MidiCommon.h>

namespace ravensnight::midi {
    
    class MidiReceiver {

        public:

            virtual void handle(const MidiEvent& event) = 0;
            friend MidiReceiver& operator<<(MidiReceiver& q, const MidiEvent& event);

    };

}


#endif // __MIDI_RECEIVER_H__