#include <Logger.h>
#include <midi/MidiStream.h>
#include <midi/MidiDevice.h>

using namespace LOGGING;
using namespace MIDI;

MidiStream::MidiStream(uint8_t intf, uint8_t cable) {
    this->cable = cable;
    this->intf = intf;
}

int MidiStream::peek() {
    return -1;
}

int MidiStream::available() {
    return tud_midi_n_available(this->intf, this->cable);
}

int MidiStream::read() {
    
    uint8_t buf[1];
    size_t len;

    if (available() > 0) {
        len = tud_midi_n_stream_read(this->intf, this->cable, buf, 1);        
        if (len == 1) {
            return buf[0];
        }
    }

    return -1;
}

size_t MidiStream::write(uint8_t b) {
    uint8_t buf[1] = { b };    

    if (!MidiDevice::instance.available()) {
        Logger::defaultLogger().debug("Do not write to midi out, since USB is not available.");
        return 0;
    }

    return tud_midi_n_stream_write(this->intf, this->cable, buf, 1);    
}

size_t MidiStream::write(const uint8_t *buf, size_t size) {
    int len = 0;

    if (!MidiDevice::instance.available()) {
        Logger::defaultLogger().debug("Do not write to midi out, since USB is not available.");
        return 0;
    }
    
    do {
        len += tud_midi_n_stream_write(this->intf, this->cable, buf + len, size - len);
        Logger::defaultLogger().debug("Stream::write - write buffer of size: %d, sent: %d", size, len);
    } while (len < size);

    return len;
}