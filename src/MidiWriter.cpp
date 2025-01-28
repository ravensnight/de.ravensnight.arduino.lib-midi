#include <esp32-hal-tinyusb.h>
#include <midi/MidiDevice.h>
#include <midi/MidiWriter.h>
#include <Logger.h>

using namespace MIDI;
using namespace LOGGING;

MidiWriter::MidiWriter(Stream* out) {  
    _stream = out;
}

void MidiWriter::send(MessageType msg, uint8_t channel, uint16_t value) {
    uint8_t lsb = __lsb(value);
    uint8_t hsb = __hsb(value);

    send(msg, channel, lsb, hsb);
}

void MidiWriter::send(MessageType type, uint8_t channel, uint8_t val1, uint8_t val2) {

    MidiMsg msg;
    uint8_t t = (uint8_t)type;
    msg.status = (uint8_t)((t & 0xF0) == 0xF0 ? t : (t | (channel & 0x0F)));
    msg.value1 = __lsb(val1);
    msg.value2 = __lsb(val2);

    switch (type) {
        case MessageType::NoteOn:
        case MessageType::NoteOff:
        case MessageType::Aftertouch:
        case MessageType::ControlChange:
        case MessageType::ModulationWheel:
        case MessageType::SongPos:
            _stream->write(__buffer(&msg), 3);
            break;

        case MessageType::ProgramChange:
        case MessageType::ChannelPressure:
        case MessageType::SongSel:
            _stream->write(__buffer(&msg), 2);
            break;
        
        case MessageType::MidiStart:
        case MessageType::MidiStop:
        case MessageType::MidiContinue:
            _stream->write(__buffer(&msg), 1);
            break;

        default:
            break;
    }
}

// ----------------------------------------------------------------------------
// voice messages
// ----------------------------------------------------------------------------
void MidiWriter::sendNoteOn(uint8_t chn, uint8_t pitch, uint8_t velocity) {
    send(MessageType::NoteOn, chn, pitch, velocity);
}

void MidiWriter::sendNoteOff(uint8_t chn, uint8_t pitch, uint8_t velocity) {
    send(MessageType::NoteOff, chn, pitch, velocity);
}

void MidiWriter::sendAftertouch(uint8_t chn, uint8_t pitch, uint8_t pressure) {
    send(MessageType::Aftertouch, chn, pitch, pressure);
}

void MidiWriter::sendControlChange(uint8_t chn, uint8_t controller, uint8_t value) {
    send(MessageType::ControlChange, chn, controller, value);
}

void MidiWriter::sendProgramSelect(uint8_t chn, uint8_t prognum) {
    send(MessageType::ProgramChange, chn, prognum, 0);
}

void MidiWriter::sendChannelPressure(uint8_t chn, uint8_t pressure) {
    send(MessageType::ChannelPressure, chn, pressure, 0);
}

void MidiWriter::sendModulationWheel(uint8_t chn, int16_t pitchValue) {
    send(MessageType::ModulationWheel, chn, pitchValue);
}

// ----------------------------------------------------------------------------
// system messages
// ----------------------------------------------------------------------------
void MidiWriter::sendSongPos(int16_t position) {
    send(MessageType::SongPos, 0, position);
}

void MidiWriter::sendSongSel(int8_t songnum) {
    send(MessageType::SongSel, 0, songnum, 0);
}   

void MidiWriter::sendMidiStart() {
    send(MessageType::MidiStart, 0, 0, 0);
}

void MidiWriter::sendMidiStop() {
    send(MessageType::MidiStop, 0, 0, 0);
}

void MidiWriter::sendMidiContinue() {
    send(MessageType::MidiContinue, 0, 0, 0);
}

void MidiWriter::sendSysEx(uint8_t channel, uint8_t payload[], uint16_t len) {
    uint16_t size = len + 3;
    uint8_t buffer[size];
    
    buffer[0] = (uint8_t)MessageType::SysExStart;
    buffer[1] = channel;
    memcpy(buffer + 2, payload, len);
    buffer[len + 2] = (uint8_t)MessageType::SysExEnd;
    
    Logger::defaultLogger().dump("Send SysEx bytes: ", buffer, size, 0);
    _stream->write(buffer, size);    
}