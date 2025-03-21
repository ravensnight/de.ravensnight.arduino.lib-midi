#ifndef __MIDI_H__
#define __MIDI_H__

#include <Arduino.h>
#include "MidiStream.h"

namespace MIDI {

#define __14bit(hsb, lsb)   ((uint16_t)((hsb & 0x7F) << 7) | (lsb & 0x7F))
#define __buffer(ptr)       ((uint8_t*)ptr)
#define __hsb(val)          ((uint8_t)(val >> 7) & 0x7F)
#define __lsb(val)          ((uint8_t)(val & 0x7F))

typedef struct _MidiMsg {
    uint8_t status;
    uint8_t value1;
    uint8_t value2;
} MidiMsg;

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

class USBPortConfig {
    public:

        int vendorId;
        int productId;
        int firmwareVersion;
        int usbPower;
        int usbVersion;
        const char* productName;
        const char* productDescription;
        const char* manufacturerName;
        const char* serial;

        USBPortConfig( int vendorId, int productId, int firmwareVersion, int usbVersion, int usbPower, const char* productName, const char* productDescription, const char* manufacturerName, const char* serial);
};

#ifndef MAX_MIDI_STREAMS
    #define MAX_MIDISTREAMS 2
#endif

class MidiDevice {

    private:

        static uint8_t stream_count;

    public:

        static MidiStream* addStream(const char* name);

        // install this interface to USB
        static void setup(const USBPortConfig& config);

};

}
#endif // __MIDI_H__