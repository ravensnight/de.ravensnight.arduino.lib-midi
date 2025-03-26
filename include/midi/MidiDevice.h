#ifndef __MIDI_H__
#define __MIDI_H__

#include <Arduino.h>
#include <Stream.h>
#include <esp32-hal-tinyusb.h>
#include <USB.h>

#include <midi/Pipe.h>

namespace MIDI {

#ifndef MAX_CABLE_COUNT
    #define MAX_CABLE_COUNT 3
#endif

#ifndef MIDI_PIPE_LEN
    #define MIDI_PIPE_LEN 64
#endif

#define __14bit(hsb, lsb)   ((uint16_t)((hsb & 0x7F) << 7) | (lsb & 0x7F))
#define __buffer(ptr)       ((uint8_t*)ptr)
#define __hsb(val)          ((uint8_t)(val >> 7) & 0x7F)
#define __lsb(val)          ((uint8_t)(val & 0x7F))

typedef struct _MidiMsg {
    uint8_t status;
    uint8_t value1;
    uint8_t value2;
} MidiMsg;

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
    int vendorId;
    int productId;
    int firmwareVersion;
    int usbVersion;
    int usbPower;
    const char* productName;
    const char* productDescription;
    const char* manufacturerName;
    const char* serial;
} USBConfig;

typedef struct{
    const char* name;
    Pipe* pipe;
} CableDef ;

class MidiDevice {

    private:

        static bool _available;
        static uint8_t cableCount;
        static uint8_t nameIndex;
        static CableDef cables[MAX_CABLE_COUNT];

        static void usbCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
        static uint16_t descriptorCallback(uint8_t * dst, uint8_t * itf);

        MidiDevice();

        // calculate the descriptor length
        static uint16_t calculateDescriptorLength();

        // get the size of bytes to read as defined in MIDI spec
        static uint8_t getPacketLen(CINType tp);
        
    public:

        static MidiDevice instance;

        // add a cable with name. return the index of the cable or -1 if entry could not be created.
        int8_t addCable(const char* cableName);

        // install this interface to USB
        void setup(const USBConfig& config);

        // check, if midi device is available.
        bool available();

        // read midi input buffer(s)
        void readInput();

        // provide the input pipe to read from.
        Pipe* getInPipe(uint8_t cable);
};

}
#endif // __MIDI_H__