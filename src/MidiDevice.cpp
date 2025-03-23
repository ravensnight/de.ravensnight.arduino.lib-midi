#include <esp32-hal-tinyusb.h>
#include <Logger.h>
#include <midi/MidiDevice.h>

using namespace MIDI;
using namespace LOGGING;

#ifndef MIDI_ENDPOINT_COUNT 
    #define MIDI_ENDPOINT_COUNT 1
#endif

ESP_EVENT_DEFINE_BASE(USB_MIDI_EVENTS);

MidiDevice::MidiDevice() {    
}

void MidiDevice::usbCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

    if (event_base == ARDUINO_USB_EVENTS) {
        arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
        switch (event_id) {
            case ARDUINO_USB_STARTED_EVENT:
                Logger::instance.info("USB PLUGGED");
                _available = true;
                break;

            case ARDUINO_USB_STOPPED_EVENT:
                Logger::instance.info("USB UNPLUGGED");
                _available = false;
                break;

            case ARDUINO_USB_SUSPEND_EVENT:
                Logger::instance.info("USB SUSPENDED: remote_wakeup_en: %d", data->suspend.remote_wakeup_en);
                _available = false;
                break;

            case ARDUINO_USB_RESUME_EVENT:
                Logger::instance.info("USB RESUMED");
                _available = true;
                break;

            default:
                break;
        }
    } 
    else if (event_base == USB_MIDI_EVENTS) {
        Logger::instance.debug("MIDI EVENT:  ID=%d, DATA=%d\r\n", event_id, (uint32_t)event_data);
    }
}

uint16_t MidiDevice::calculateDescriptorLength() {
    return TUD_MIDI_DESC_HEAD_LEN + \
           TUD_MIDI_DESC_JACK_LEN * cableCount + \
           TUD_MIDI_DESC_EP_LEN(cableCount) * 2;
} 

uint16_t MidiDevice::descriptorCallback(uint8_t * dst, uint8_t * itf) {

    uint16_t len = calculateDescriptorLength();
    uint16_t pos = 0;

    // product name = index 4
    uint8_t head[] = { TUD_MIDI_DESC_HEAD(*itf, 4, cableCount) };
    memcpy(dst, head, TUD_MIDI_DESC_HEAD_LEN);
    pos += TUD_MIDI_DESC_HEAD_LEN;

    // add jack descriptors
    for (uint8_t i = 0; i < cableCount; i++) {
        uint8_t array1[] = { TUD_MIDI_DESC_JACK_DESC(i + 1, (uint8_t)(nameIndex + i)) };
        memcpy(dst + pos, array1, sizeof(array1));        
        pos += sizeof(array1);
    }
    
    // add inbound endpoint descriptors
    uint8_t desc1[] = { TUD_MIDI_DESC_EP(MIDI_ENDPOINT_COUNT, 64, cableCount) };
    memcpy(dst + pos, desc1 , sizeof(desc1));
    pos += sizeof(desc1);

    for (uint8_t i = 0; i < cableCount; i++) {
        uint8_t jackIn[] = { TUD_MIDI_JACKID_IN_EMB(i + 1) };
        memcpy(dst + pos, jackIn, sizeof(jackIn));
        pos += sizeof(jackIn);
    }

    // add outbound endpoint descriptors
    uint8_t desc2[] = { TUD_MIDI_DESC_EP(0x80 | MIDI_ENDPOINT_COUNT, 64, cableCount) };
    memcpy(dst + pos, desc2 , sizeof(desc2));
    pos += sizeof(desc2);

    for (uint8_t i = 0; i < cableCount; i++) {
        uint8_t jackOut[] = { TUD_MIDI_JACKID_OUT_EMB(i + 1) };
        memcpy(dst + pos, jackOut, sizeof(jackOut));
        pos += sizeof(jackOut);
    }

    *itf += 2;  // +2 interface count(Audio Control, Midi Streaming)
    return len;
}

int8_t MidiDevice::addCable(const char* name) {
    if (cableCount < MAX_CABLE_COUNT) {
        cableNames[cableCount] = name;
        cableCount++;

        return (cableCount - 1);
    }

    return -1;
}

void MidiDevice::setup(const USBConfig& config) {

    // Change USB Device Descriptor Parameter
    USB.VID(config.vendorId); // vendor id
    USB.PID(config.productId);    
    USB.firmwareVersion(config.firmwareVersion);
    USB.usbPower(config.usbPower);
    USB.usbVersion(config.usbVersion);
    USB.usbClass(TUSB_CLASS_AUDIO);
    USB.usbSubClass(0x00);
    USB.usbProtocol(0x00);
    USB.usbAttributes(0x80);    
    USB.serialNumber(config.serial == 0 ? "0000" : config.serial);
    USB.productName(config.productName == 0 ? "Midi Device" : config.productName);
    USB.manufacturerName(config.manufacturerName == 0 ? "SynthHead" : config.manufacturerName);
    
    if (config.productDescription != 0) {
        nameIndex = tinyusb_add_string_descriptor(config.productDescription);
    } else {
        nameIndex = tinyusb_add_string_descriptor("Midi Device Description");
    }

    nameIndex++;
    for (uint8_t i = 0; i < cableCount; i++) {
        tinyusb_add_string_descriptor(cableNames[i]);
    }

    tinyusb_enable_interface(USB_INTERFACE_MIDI, calculateDescriptorLength(), descriptorCallback);
    USB.onEvent(MidiDevice::usbCallback);        

    USB.begin();

    // available flag is being set in USB callback function.
}

bool MidiDevice::available() {
    return _available;
}

MidiDevice MidiDevice::instance = MidiDevice();

bool MidiDevice::_available = false;
uint8_t MidiDevice::cableCount = 0;
uint8_t MidiDevice::nameIndex = 0;
const char* MidiDevice::cableNames[MAX_CABLE_COUNT] = { 0 };
