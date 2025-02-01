#include <esp32-hal-tinyusb.h>
#include <Logger.h>
#include <midi/MidiReader.h>

using namespace MIDI;
using namespace LOGGING;

MidiReader::MidiReader(Stream* mio, MidiCallback* cb) {    
    _midiCallback = cb;
    _stream = mio;
}

uint8_t MidiReader::payloadSize(MessageType type) {
    switch (type) {
        case MessageType::NoteOn:
        case MessageType::NoteOff:
        case MessageType::Aftertouch:
        case MessageType::ControlChange:
        case MessageType::ModulationWheel:
        case MessageType::SongPos:
            return 2;
    
        case MessageType::ProgramChange:
        case MessageType::ChannelPressure:
        case MessageType::SongSel:
        case MessageType::SysExStart:
            return 1;

        case MessageType::MidiStart:
        case MessageType::MidiContinue:
        case MessageType::MidiStop:
        case MessageType::SysExEnd:
        case MessageType::Reset:
        default:
            return 0;

    }
}

void MidiReader::parse() {
    MidiMsg msg;
    
    uint8_t len;
    int c;
    
    if (_state == ReaderState::WaitStart) {

        c = _stream->read();
        if (c < 0) return;

        msg.status = c;
        if (msg.status < 0xF0) {
            _command = (MessageType)(msg.status & 0xF0);
            _channel = (msg.status & 0x0F);
        } else {
            _channel = 0;
            _command = (MessageType)msg.status;
        }

        _state = ReaderState::WaitPayload;
    }
    else if (_state == ReaderState::WaitPayload) {
        
        len = payloadSize(_command);
        if (_stream->readBytes(__buffer(&msg + 1), len) == len) {

            switch (_command) {
            case MessageType::NoteOn:
                _midiCallback->onNoteOn(_channel, msg.value1, msg.value2);
                break;

            case MessageType::NoteOff:
                _midiCallback->onNoteOff(_channel, msg.value1, msg.value2);
                break;

            case MessageType::Aftertouch:
                _midiCallback->onAftertouch(_channel, msg.value1, msg.value2);
                break;

            case MessageType::ControlChange:
                _midiCallback->onControlChange(_channel, msg.value1, msg.value2);
                break;

            case MessageType::ProgramChange:
                _midiCallback->onProgramSelect(_channel, msg.value1);
                break;

            case MessageType::ChannelPressure:
                _midiCallback->onChannelPressure(_channel, msg.value1);
                break;

            case MessageType::ModulationWheel:
                _midiCallback->onModulationWheel(_channel, __14bit(msg.value2, msg.value1));
                break;

            case MessageType::SongPos:                
                _midiCallback->onSongPos(__14bit(msg.value2, msg.value1));
                break;

            case MessageType::SongSel:
                _midiCallback->onSongSel(msg.value1);
                break;
            
            case MessageType::MidiStart:
                _midiCallback->onMidiStart();
                break;

            case MessageType::MidiStop:
                _midiCallback->onMidiStop();
                break;

            case MessageType::MidiContinue:
                _midiCallback->onMidiContinue();
                break;

            case MessageType::SysExStart:
                Logger::instance.debug("Received SysEx for Manufacturer: %x", msg.value1);
                _midiCallback->onSysEx(msg.value1, this);
                break;

            case MessageType::SysExEnd:
                Logger::instance.debug("Received SysExEnd.");
                break;

            default:
                Logger::instance.debug("Unhandled command: %x. Skip.", _command);
                break;
            }     
        }

        _state = ReaderState::WaitStart;
        _command = MessageType::Reset;
        _channel = 0x00;
    }
}

uint16_t MidiReader::readSysEx(uint8_t buffer[], uint16_t len) {
    
    uint8_t buf[1];
    for (uint16_t i = 0; i < len; i++) {
        if (_stream->readBytes(buf, 1) > 0) {
            if ((buf[0] & 0x80) == 0x80) {
                Logger::instance.warn("Received end of stream signal %x at position %d. Stop here.", buf[0], i);
                return i;
            }
        }

        // no more bytes OR end of sysex OR invalid
        buffer[i] = buf[0];
    }

    Logger::instance.dump("Received SysEx bytes:  ", buffer, len, 0);
    return len;
}

void MidiReader::skipSysEx() {
    uint8_t b[1];
    while (_stream->readBytes(b, 1) > 0) {
        Logger::instance.debug("Skip sysex byte: 0x%x", b[0]);
    }
}
