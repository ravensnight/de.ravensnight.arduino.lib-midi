#ifndef __MIDI_STREAM_H__
#define __MIDI_STREAM_H__

#include <Stream.h>

#include "Adafruit_TinyUSB.h"
// #include "tusb.h"

namespace MIDI {

    class MidiStream : public Stream {

        private:

            Adafruit_USBD_MIDI* midiPort = 0;;

        public:

            MidiStream(const char* portName);

            int available();
            int read();
            int peek();

            size_t write(uint8_t b);
            size_t write(const uint8_t *buffer, size_t size);
    };

}

#endif // __MIDI_STREAM_H__