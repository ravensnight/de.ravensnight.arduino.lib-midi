#include <midi/VoiceReceiver.h>
#include <Logger.h>
#include <async/LockGuard.h>

using namespace ravensnight::logging;
using namespace ravensnight::midi;

VoiceReceiver::VoiceReceiver(VoiceCallback* cb) : _mutex("VoiceReceiver") {
    _cb = cb;
}

VoiceReceiver::~VoiceReceiver() {
    _cb = 0;
}

bool VoiceReceiver::accepted(CINType type) {

    switch (type) {
        case CINType::Common2Byte:
        case CINType::Common3Byte:
        case CINType::ProgramChange:
        case CINType::ChannelPressure:
        case CINType::NoteOff:
        case CINType::NoteOn:
        case CINType::PolyKey:
        case CINType::ControlChange:
        case CINType::ModulationWheel:
        case CINType::SingleByte:
            return true;

        default:
            // CINType::Misc
            // CINType::CableEvent
            return 0;            
    }    
}

void VoiceReceiver::handle(const MidiEvent& evt) {
    synchronized(_mutex);

    if ((_cb == 0) || (!accepted(evt.type)) || (evt.msgLength < 3)) {
        Logger::warn("Cannot handle message: type=%d, len=%d", evt.type, evt.msgLength);
        return;
    }

    uint8_t status = evt.msg[0];
    uint8_t value1 = evt.msg[1];
    uint8_t value2 = evt.msg[2];

    MessageType command = MessageType::Reset;
    uint8_t channel = 0;
    
    if (status < 0xF0) {
        command = (MessageType)(status & 0xF0);
        channel = (status & 0x0F);
    } else {
        channel = 0;
        command = (MessageType)status;
    }

    switch (command) {
    case MessageType::NoteOn:
        _cb->onNoteOn(channel, value1, value2);
        break;

    case MessageType::NoteOff:
        _cb->onNoteOff(channel, value1, value2);
        break;

    case MessageType::Aftertouch:
        _cb->onAftertouch(channel, value1, value2);
        break;

    case MessageType::ControlChange:
        _cb->onControlChange(channel, value1, value2);
        break;

    case MessageType::ProgramChange:
        _cb->onProgramSelect(channel, value1);
        break;

    case MessageType::ChannelPressure:
        _cb->onChannelPressure(channel, value1);
        break;

    case MessageType::ModulationWheel:
        _cb->onModulationWheel(channel, __14bit(value2, value1));
        break;

    case MessageType::SongPos:                
        _cb->onSongPos(__14bit(value2, value1));
        break;

    case MessageType::SongSel:
        _cb->onSongSel(value1);
        break;
    
    case MessageType::MidiStart:
        _cb->onMidiStart();
        break;

    case MessageType::MidiStop:
        _cb->onMidiStop();
        break;

    case MessageType::MidiContinue:
        _cb->onMidiContinue();
        break;

    default:
        Logger::debug("Unhandled voice command: %x. Skip.", command);
        break;
    }     
}
