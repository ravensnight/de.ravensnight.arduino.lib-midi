#ifndef __MIDI_STREAM_H__
#define __MIDI_STREAM_H__

#include <Stream.h>

namespace MIDI {

    class MidiStream : public Stream {

        private:
            uint8_t cable, intf;

        public:

            MidiStream(uint8_t intf, uint8_t cable);

            int available();
            int read();
            int peek();

            size_t write(uint8_t b);
            size_t write(const uint8_t *buffer, size_t size);
    };

}

#endif // __MIDI_STREAM_H__