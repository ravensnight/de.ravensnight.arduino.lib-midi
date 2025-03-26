#include <esp32-hal-tinyusb.h>
#include <Logger.h>
#include <midi/MidiReader.h>

using namespace MIDI;
using namespace LOGGING;

MidiReader::MidiReader(Stream* mio) {        
    _stream = mio;
}

void MidiReader::enableSysex(SysexHandler* handler) {
    _sysexHandler = handler;
}

void MidiReader::enableVoice(VoiceCallback* cb) {
    _voiceCallback = cb;
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

        Logger::instance.debug("Read command is: %x", _command);
        _state = ReaderState::WaitPayload;
    }
    else if (_state == ReaderState::WaitPayload) {
        
        len = payloadSize(_command);
        Logger::instance.debug("Try to read payload for command %x, len: %d", _command, len);

        if (_stream->readBytes(__buffer(&msg + 1), len) == len) {
            
            Logger::instance.debug("Read payload for command %x, len: %d", _command, len);
            Logger::instance.dump("Command incl. payload: ", (uint8_t*)&msg, len + 1, len + 1);

            switch (_command) {
            case MessageType::NoteOn:
                run_if_notnull(_voiceCallback, onNoteOn(_channel, msg.value1, msg.value2));
                break;

            case MessageType::NoteOff:
                run_if_notnull(_voiceCallback, onNoteOff(_channel, msg.value1, msg.value2));
                break;

            case MessageType::Aftertouch:
                run_if_notnull(_voiceCallback, onAftertouch(_channel, msg.value1, msg.value2));
                break;

            case MessageType::ControlChange:
                run_if_notnull(_voiceCallback, onControlChange(_channel, msg.value1, msg.value2));
                break;

            case MessageType::ProgramChange:
                run_if_notnull(_voiceCallback, onProgramSelect(_channel, msg.value1));
                break;

            case MessageType::ChannelPressure:
                run_if_notnull(_voiceCallback, onChannelPressure(_channel, msg.value1));
                break;

            case MessageType::ModulationWheel:
                run_if_notnull(_voiceCallback, onModulationWheel(_channel, __14bit(msg.value2, msg.value1)));
                break;

            case MessageType::SongPos:                
                run_if_notnull(_voiceCallback, onSongPos(__14bit(msg.value2, msg.value1)));
                break;

            case MessageType::SongSel:
                run_if_notnull(_voiceCallback, onSongSel(msg.value1));
                break;
            
            case MessageType::MidiStart:
                run_if_notnull(_voiceCallback, onMidiStart());
                break;

            case MessageType::MidiStop:
                run_if_notnull(_voiceCallback, onMidiStop());
                break;

            case MessageType::MidiContinue:
                run_if_notnull(_voiceCallback, onMidiContinue());
                break;

            case MessageType::SysExStart:
                Logger::instance.debug("Received SysEx for Manufacturer: %x", msg.value1);
                run_if_notnull(_sysexHandler, onSysEx(msg.value1, this));
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
