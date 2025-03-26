#include <midi/Pipe.h>
#include <Logger.h>

using namespace LOGGING;
using namespace MIDI;

Pipe::Pipe(uint16_t size) {
    this->buffer = (uint8_t*)malloc(size);
    memset(this->buffer, 0, size);

    this->size = size;
    this->rp = size; // set read pointer to "invalid"
    this->wp = size; // set write pointer to "invalid"
}

Pipe::~Pipe() {
    delete this->buffer;
}

// return 0 if pipe is empty or the number of bytes available for read
uint16_t Pipe::available() {
    if (wp == rp) {
        return 0;
    } 
    else if (rp < wp) {
        return wp - rp;
    }
    else {  // wp flipped already
        return (size - rp) + wp;
    }
}

// add a single byte to pipe
void Pipe::add(uint8_t byte) {
    if (wp == size) {
        wp = 0;
    }

    if (rp == wp) {
        Logger::instance.warn("Overwrite data at current read position: %d", rp);
    }

    buffer[wp] = byte;
    wp++;
}

// multiple bytes 
void Pipe::add(const uint8_t* source, uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
        add(source[i]);
    }

    // Logger::instance.debug("Update buffer. read: %d, write: %d", rp, wp);
    // Logger::instance.dump("New buffer: ", this->buffer, this->size, 8);    
}

// read a single byte from buffer or return -1, if nothing is to read.
int16_t Pipe::read() {
    if (rp == size) {
        rp = 0;
    }

    if (wp == rp) {
        Logger::instance.warn("No data available in buffer: %d", rp);
        return -1;
    }

    int16_t res = buffer[rp];

    // Logger::instance.debug("Read buffer at position: %d, value:%04x", rp, res);
    // Logger::instance.dump("Read from buffer: ", this->buffer, this->size, 8);    

    rp++;

    return res;
}

// read multipe bytes into given buffer, return the number of bytes that where available.
uint16_t Pipe::read(uint8_t buffer[], uint16_t len) {
    uint16_t r = 0;
    int16_t d;

    while (r < len) {
        d = read();
        if (d < 0) break;

        buffer[r] = (uint8_t)(d & 0xFF);
        r++;
    }

    return r;
}
