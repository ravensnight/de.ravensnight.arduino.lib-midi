#include <esp32-hal-tinyusb.h>
#include <midi/MidiDevice.h>
#include <midi/MidiTransmitter.h>
#include <Masquerade.h>
#include <Logger.h>

using namespace MIDI;
using namespace LOGGING;

MidiTransmitter::MidiTransmitter(uint8_t cable, size_t bufferSize) {  
    _cable = cable;
    _outBufferSize = bufferSize;
    _outBuffer = (uint8_t*)malloc(bufferSize);
}

MidiTransmitter::MidiTransmitter(MidiTransmitter& source) {  
    _cable = source._cable;
    _outBufferSize = source._outBufferSize;
    _outBuffer = (uint8_t*)malloc(_outBufferSize);
}

MidiTransmitter::~MidiTransmitter() {    
    free(_outBuffer);
}

size_t MidiTransmitter::write(const uint8_t *buf, size_t size) {
    int len = 0;

    if (!MidiDevice::instance.available()) {
        Logger::debug("Do not write to midi out, since USB is not available.");
        return 0;
    }
    
    do {
        len += tud_midi_n_stream_write(0, this->_cable, buf + len, size - len);
        Logger::debug("Stream::write - write buffer of size: %d, sent: %d", size, len);
    } while (len < size);

    return len;
}

void MidiTransmitter::send(MessageType type, uint8_t channel, uint8_t val1, uint8_t val2) {

    uint8_t t = (uint8_t)type;
    uint8_t msg[3];

    msg[0] = (uint8_t)((t & 0xF0) == 0xF0 ? t : (t | (channel & 0x0F)));
    msg[1] = __lsb(val1);
    msg[2] = __lsb(val2);

    switch (type) {
        case MessageType::NoteOn:
        case MessageType::NoteOff:
        case MessageType::Aftertouch:
        case MessageType::ControlChange:
        case MessageType::ModulationWheel:
        case MessageType::SongPos:
            write(msg, 3);
            break;

        case MessageType::ProgramChange:
        case MessageType::ChannelPressure:
        case MessageType::SongSel:
            write(msg, 2);
            break;
        
        case MessageType::MidiStart:
        case MessageType::MidiStop:
        case MessageType::MidiContinue:
            write(msg, 1);
            break;

        default:
            break;
    }
}

void MidiTransmitter::send(MessageType msg, uint8_t channel, uint16_t value) {
    uint8_t lsb = __lsb(value);
    uint8_t hsb = __hsb(value);

    send(msg, channel, lsb, hsb);
}

// ----------------------------------------------------------------------------
// voice messages
// ----------------------------------------------------------------------------
void MidiTransmitter::sendNoteOn(uint8_t chn, uint8_t pitch, uint8_t velocity) {
    send(MessageType::NoteOn, chn, pitch, velocity);
}

void MidiTransmitter::sendNoteOff(uint8_t chn, uint8_t pitch, uint8_t velocity) {
    send(MessageType::NoteOff, chn, pitch, velocity);
}

void MidiTransmitter::sendAftertouch(uint8_t chn, uint8_t pitch, uint8_t pressure) {
    send(MessageType::Aftertouch, chn, pitch, pressure);
}

void MidiTransmitter::sendControlChange(uint8_t chn, uint8_t controller, uint8_t value) {
    send(MessageType::ControlChange, chn, controller, value);
}

void MidiTransmitter::sendProgramSelect(uint8_t chn, uint8_t prognum) {
    send(MessageType::ProgramChange, chn, prognum, 0);
}

void MidiTransmitter::sendChannelPressure(uint8_t chn, uint8_t pressure) {
    send(MessageType::ChannelPressure, chn, pressure, 0);
}

void MidiTransmitter::sendModulationWheel(uint8_t chn, int16_t pitchValue) {
    send(MessageType::ModulationWheel, chn, pitchValue);
}

// ----------------------------------------------------------------------------
// system messages
// ----------------------------------------------------------------------------
void MidiTransmitter::sendSongPos(int16_t position) {
    send(MessageType::SongPos, 0, position);
}

void MidiTransmitter::sendSongSel(int8_t songnum) {
    send(MessageType::SongSel, 0, songnum, 0);
}   

void MidiTransmitter::sendMidiStart() {
    send(MessageType::MidiStart, 0, 0, 0);
}

void MidiTransmitter::sendMidiStop() {
    send(MessageType::MidiStop, 0, 0, 0);
}

void MidiTransmitter::sendMidiContinue() {
    send(MessageType::MidiContinue, 0, 0, 0);
}

size_t MidiTransmitter::sendSysEx(uint8_t channel, uint8_t payload[], uint16_t len, Converter& converter) {
    std::lock_guard<std::mutex> lock(bufferLock);

    size_t payloadSize = converter.getEncodedSize(len);
    uint16_t size = payloadSize + 3;

    if (size > _outBufferSize) {
        Logger::error("MidiTransmitter::sendSysEx - outbuffer size %d exceeded by payload size %d(%d). Cannot send sysex!", _outBufferSize, size, len);
        return -1;
    }
    
    _outBuffer[0] = (uint8_t)MessageType::SysExStart;
    _outBuffer[1] = channel;

    converter.encode(_outBuffer + 2, payloadSize, payload, len);
    _outBuffer[payloadSize + 2] = (uint8_t)MessageType::SysExEnd;
    
    Logger::dump("Send SysEx bytes: ", _outBuffer, size, 0);
    write(_outBuffer, size);    

    return size;
}

size_t MidiTransmitter::sendSysEx(uint8_t channel, uint8_t payload[], uint16_t len) {
    Masquerade conv = Masquerade();
    return sendSysEx(channel, payload, len, conv);
}
