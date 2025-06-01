#ifndef __MIDI_COMMON_H__
#define __MIDI_COMMON_H__

#include <Arduino.h>

namespace MIDI {

    #define __14bit(hsb, lsb)   ((uint16_t)((hsb & 0x7F) << 7) | (lsb & 0x7F))
    #define __buffer(ptr)       ((uint8_t*)ptr)
    #define __hsb(val)          ((uint8_t)(val >> 7) & 0x7F)
    #define __lsb(val)          ((uint8_t)(val & 0x7F))
    #define run_if_notnull(ptr, fn) if (ptr != 0) ptr->fn;

    enum class CINType : uint8_t {

        Misc = 0x00,
        CableEvent = 0x01,
        Common2Byte = 0x02,
        Common3Byte = 0x03,
        SysexStart = 0x04,
        SysexEnd1 = 0x05,
        SysexEnd2 = 0x06,
        SysexEnd3 = 0x07,
        NoteOff = 0x08,
        NoteOn = 0x09,
        PolyKey = 0x0A,
        ControlChange = 0x0B,
        ProgramChange = 0x0C,
        ChannelPressure = 0x0D,
        ModulationWheel = 0x0E,
        SingleByte = 0x0F    
    };

    enum class MessageType : uint8_t {

        // voice messages
        NoteOn = 0x90,
        NoteOff = 0x80,
        Aftertouch = 0xA0,  // key pressure
        ControlChange = 0xB0,
        ProgramChange = 0xC0,
        ChannelPressure = 0xD0,
        ModulationWheel = 0xE0,
    
        // system messages
        SysExStart = 0xF0,
        SysExEnd = 0xF7,
        MidiTime = 0xF1,
        SongPos = 0xF2,
        SongSel = 0xF3,
        TuneReq = 0xF6,
        MidiSync = 0xF8,    // midi clock
        MidiStart = 0xFA,
        MidiContinue = 0xFB,
        MidiStop = 0xFC,
        ActiveSens = 0xFE,
        Reset = 0xFF    
    };

    typedef struct {
        CINType type;
        uint8_t cable;
        uint8_t msg[3];        
        uint8_t msgLength;
    } MidiEvent;

}


#endif // __MIDI_COMMON_H__