#include <midi/VoiceReceiver.h>
#include <Logger.h>

using namespace LOGGING;
using namespace MIDI;

VoiceReceiver::VoiceReceiver(VoiceCallback* cb) {
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

void VoiceReceiver::handle(CINType type, const uint8_t* buffer, size_t len) {

    if ((_cb == 0) || (!accepted(type))) return;

    MessageType command = MessageType::Reset;
    uint8_t channel = 0;
    
    MidiMsg msg = { .status = 0, .value1 = 0, .value2 = 0 };
    size_t l = (len > 3) ? 3 : len;

    memcpy(&msg, buffer, l);

    if (msg.status < 0xF0) {
        command = (MessageType)(msg.status & 0xF0);
        channel = (msg.status & 0x0F);
    } else {
        channel = 0;
        command = (MessageType)msg.status;
    }

    switch (command) {
    case MessageType::NoteOn:
        _cb->onNoteOn(channel, msg.value1, msg.value2);
        break;

    case MessageType::NoteOff:
        _cb->onNoteOff(channel, msg.value1, msg.value2);
        break;

    case MessageType::Aftertouch:
        _cb->onAftertouch(channel, msg.value1, msg.value2);
        break;

    case MessageType::ControlChange:
        _cb->onControlChange(channel, msg.value1, msg.value2);
        break;

    case MessageType::ProgramChange:
        _cb->onProgramSelect(channel, msg.value1);
        break;

    case MessageType::ChannelPressure:
        _cb->onChannelPressure(channel, msg.value1);
        break;

    case MessageType::ModulationWheel:
        _cb->onModulationWheel(channel, __14bit(msg.value2, msg.value1));
        break;

    case MessageType::SongPos:                
        _cb->onSongPos(__14bit(msg.value2, msg.value1));
        break;

    case MessageType::SongSel:
        _cb->onSongSel(msg.value1);
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
        Logger::instance.debug("Unhandled voice command: %x. Skip.", command);
        break;
    }     
}
