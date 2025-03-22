#include <Logger.h>
#include <midi/MidiStream.h>
#include <midi/MidiDevice.h>

using namespace LOGGING;
using namespace MIDI;

MidiStream::MidiStream(const char* portName) {
    midiPort = new Adafruit_USBD_MIDI();
    if (portName != 0) {
        midiPort->setStringDescriptor(portName);
    }

    midiPort->begin();
}

int MidiStream::peek() {
    return midiPort->peek();
}

int MidiStream::available() {    
    return midiPort->available();
}

int MidiStream::read() {
    return midiPort->read();
}

size_t MidiStream::write(uint8_t b) {
    if (midiPort->availableForWrite()) {
        return midiPort->write(b);
    }

    return 0;
}

size_t MidiStream::write(const uint8_t *buf, size_t size) {
    int len = 0;

    if (!midiPort->availableForWrite()) {
        Logger::instance.debug("Do not write to midi out, since USB is not available.");
        return 0;
    }
    
    do {
        len += midiPort->write(buf + len, size - len);
        Logger::instance.debug("Stream::write - write buffer of size: %d, sent: %d", size, len);
    } while (len < size);

    return len;
}
