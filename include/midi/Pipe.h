#ifndef __PIPE_H__
#define __PIPE_H__

#include <Arduino.h>

namespace MIDI {

    class Pipe {

        private:
            uint8_t* buffer;
            uint16_t size;

            uint16_t rp;    // read pointer
            uint16_t wp;    // write pointer;

        public:

            Pipe(uint16_t size);
            ~Pipe();

            // return 0 if pipe is empty or the number of bytes available for read
            uint16_t available();

            // add a single byte to pipe
            void add(uint8_t byte);

            // multiple bytes 
            void add(const uint8_t* buffer, uint16_t size);

            // read a single byte from buffer or return -1, if nothing is to read.
            int16_t read();

            // read multipe bytes into given buffer, return the number of bytes that where available.
            uint16_t read(uint8_t buffer[], uint16_t size);
    };

}
#endif // __PIPE_H__