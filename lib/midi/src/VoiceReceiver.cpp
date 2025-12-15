#include <cassert>

#include <midi/LoggerConfig.h>
#include <midi/VoiceReceiver.h>
#include <async/LockGuard.h>

using namespace ravensnight::logging;
using namespace ravensnight::midi;
using namespace ravensnight::async;

VoiceReceiver::VoiceReceiver(Ref<VoiceCallback>& cb) : 
    _mutex("VoiceReceiver"),
    _cb(cb)
{
    assert(!cb.isNULL());
}

VoiceReceiver::~VoiceReceiver() {
    _cb.clear();
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
            return false;            
    }    
}

void VoiceReceiver::handle(const MidiEvent& evt) {
    acquirelock(_mutex);

    if ((!accepted(evt.type)) || (evt.msgLength < 3)) {
        _logger.warn("Cannot handle message: type=%d, len=%d", evt.type, evt.msgLength);
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

    VoiceCallback* cb = _cb.get();

    switch (command) { 
    case MessageType::NoteOn:
        cb->onNoteOn(channel, value1, value2);
        break;

    case MessageType::NoteOff:
        cb->onNoteOff(channel, value1, value2);
        break;

    case MessageType::Aftertouch:
        cb->onAftertouch(channel, value1, value2);
        break;

    case MessageType::ControlChange:
        cb->onControlChange(channel, value1, value2);
        break;

    case MessageType::ProgramChange:
        cb->onProgramSelect(channel, value1);
        break;

    case MessageType::ChannelPressure:
        cb->onChannelPressure(channel, value1);
        break;

    case MessageType::ModulationWheel:
        cb->onModulationWheel(channel, __14bit(value2, value1));
        break;

    case MessageType::SongPos:                
        cb->onSongPos(__14bit(value2, value1));
        break;

    case MessageType::SongSel:
        cb->onSongSel(value1);
        break;
    
    case MessageType::MidiStart:
        cb->onMidiStart();
        break;

    case MessageType::MidiStop:
        cb->onMidiStop();
        break;

    case MessageType::MidiContinue:
        cb->onMidiContinue();
        break;

    default:
        _logger.debug("Unhandled voice command: %x. Skip.", command);
        break;
    }     
}

ClassLogger VoiceReceiver::_logger(LC_MIDI_VOICE);