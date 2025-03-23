#ifndef __MIDI_H__
#define __MIDI_H__

#include <Arduino.h>
#include <Stream.h>

#include <esp32-hal-tinyusb.h>
#include <USB.h>

namespace MIDI {

#ifndef MAX_CABLE_COUNT
    #define MAX_CABLE_COUNT 3
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

class MidiDevice {

    private:

        static bool _available;
        static  uint8_t cableCount;
        static  const char* cableNames[MAX_CABLE_COUNT];

        static void usbCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
        static uint16_t descriptorCallback(uint8_t * dst, uint8_t * itf);

        MidiDevice();

        // calculate the descriptor length
        static uint16_t calculateDescriptorLength();

    public:

        static MidiDevice instance;

        // add a cable with name. return the index of the cable or -1 if entry could not be created.
        int8_t addCable(const char* cableName);

        // install this interface to USB
        void setup(const USBConfig& config);

        // check, if midi device is available.
        bool available();
};

}
#endif // __MIDI_H__