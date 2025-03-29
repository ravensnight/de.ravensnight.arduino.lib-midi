#ifndef __MIDI_RECEIVER_H__
#define __MIDI_RECEIVER_H__

#include <Arduino.h>
#include <midi/MidiCommon.h>

namespace MIDI {
    
    class MidiReceiver {

        public:
            virtual void handle(CINType type, const uint8_t* msg, size_t len) = 0;
    };

}


#endif // __MIDI_RECEIVER_H__