#ifndef __MIDI_H__
#define __MIDI_H__

#include <Arduino.h>
#include <Stream.h>
#include <esp32-hal-tinyusb.h>
#include <USB.h>

#include <async/Mutex.h>
#include <midi/MidiCommon.h>
#include <midi/MidiReceiver.h>

using namespace ravensnight::async;

namespace ravensnight::midi {

#define MAX_CABLE_NAMELEN 25
#ifndef MAX_CABLE_COUNT
    #define MAX_CABLE_COUNT 3
#endif

#ifndef MIDI_PIPE_LEN
    #define MIDI_PIPE_LEN 64
#endif
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

typedef struct {
    char name[MAX_CABLE_NAMELEN + 1] = { 0 };
    MidiReceiver* receiver = 0;
} CableDef ;

class MidiDevice {

    private:

        static uint8_t cableCount;
        static CableDef cables[MAX_CABLE_COUNT];

        static bool _available;
        static uint8_t nameIndex;
        static uint8_t _packet[4];

        static void usbCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        static uint16_t descriptorCallback(uint8_t* dst, uint8_t * itf);

        MidiDevice();
        ~MidiDevice();

        // calculate the descriptor length
        static uint16_t calculateDescriptorLength();

        // get the size of bytes to read as defined in MIDI spec
        static uint8_t getPacketLen(CINType tp);
        
    public:

        static MidiDevice instance;

        // add a cable with name. return the index of the cable or -1 if entry could not be created.
        int8_t attach(const char* cableName, MidiReceiver* q);

        // install this interface to USB
        void setup(const USBConfig& config);

        // check, if midi device is available.
        bool available();

        // read midi input into given queue. 
        void readInput();

};

}
#endif // __MIDI_H__