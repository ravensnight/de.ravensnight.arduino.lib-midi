#include <Logger.h>
#include <midi/MidiStream.h>
#include <midi/MidiDevice.h>

using namespace LOGGING;
using namespace MIDI;

MidiStream::MidiStream(uint8_t cable) {
    this->cable = cable;
}

int MidiStream::peek() {
    return -1;
}

int MidiStream::available() {

    Pipe* pipe = MidiDevice::instance.getInPipe(this->cable);
    if (pipe == 0) return 0;

    return pipe->available();
}

int MidiStream::read() {
    Pipe* pipe = MidiDevice::instance.getInPipe(this->cable);
    if (pipe == 0) return -1;

    if (pipe->available() > 0) {        
        int res = pipe->read();

        // Logger::instance.debug("Read from cable pipe %d: %04x", this->cable, res);
        return res;
    }
    
    return -1;
}

size_t MidiStream::write(uint8_t b) {
    uint8_t buf[1] = { b };    

    if (!MidiDevice::instance.available()) {
        Logger::instance.debug("Do not write to midi out, since USB is not available.");
        return 0;
    }

    return tud_midi_n_stream_write(0, this->cable, buf, 1);    
}

size_t MidiStream::write(const uint8_t *buf, size_t size) {
    int len = 0;

    if (!MidiDevice::instance.available()) {
        Logger::instance.debug("Do not write to midi out, since USB is not available.");
        return 0;
    }
    
    do {
        len += tud_midi_n_stream_write(0, this->cable, buf + len, size - len);
        Logger::instance.debug("Stream::write - write buffer of size: %d, sent: %d", size, len);
    } while (len < size);

    return len;
}