#include <esp32-hal-tinyusb.h>
#include <Logger.h>
#include <midi/MidiDevice.h>

using namespace MIDI;
using namespace LOGGING;

#ifndef MIDI_ENDPOINT_COUNT 
    #define MIDI_ENDPOINT_COUNT 1
#endif

#ifndef CABLE_COUNT 
    #define CABLE_COUNT 1
#endif 

#define TUSB_MIDI_DESCRIPTOR_LEN (  \
    TUD_MIDI_DESC_HEAD_LEN + \
    TUD_MIDI_DESC_JACK_LEN * CABLE_COUNT + \
    TUD_MIDI_DESC_EP_LEN(CABLE_COUNT) * 2 )

ESP_EVENT_DEFINE_BASE(USB_MIDI_EVENTS);

MidiDevice::MidiDevice() {    
    _available = false;
}

void MidiDevice::usbCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

    if (event_base == ARDUINO_USB_EVENTS) {
        arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
        switch (event_id) {
            case ARDUINO_USB_STARTED_EVENT:
                Logger::defaultLogger().info("USB PLUGGED");
                MidiIO._available = true;
                break;

            case ARDUINO_USB_STOPPED_EVENT:
                Logger::defaultLogger().info("USB UNPLUGGED");
                MidiIO._available = false;
                break;

            case ARDUINO_USB_SUSPEND_EVENT:
                Logger::defaultLogger().info("USB SUSPENDED: remote_wakeup_en: %d", data->suspend.remote_wakeup_en);
                MidiIO._available = false;
                break;

            case ARDUINO_USB_RESUME_EVENT:
                Logger::defaultLogger().info("USB RESUMED");
                MidiIO._available = true;
                break;

            default:
                break;
        }
    } 
    else if (event_base == USB_MIDI_EVENTS) {
        Logger::defaultLogger().debug("MIDI EVENT:  ID=%d, DATA=%d\r\n", event_id, (uint32_t)event_data);
    }
}

uint16_t MidiDevice::descriptorCallback(uint8_t * dst, uint8_t * itf) {

    uint8_t descriptor[TUSB_MIDI_DESCRIPTOR_LEN] = {
        TUD_MIDI_DESC_HEAD(*itf, 4, CABLE_COUNT),           // header

        // jack descriptors
        TUD_MIDI_DESC_JACK_DESC(1, 0),                      // cable 1
        // TUD_MIDI_DESC_JACK_DESC(2, 0),                   // cable 2
        // TUD_MIDI_DESC_JACK_DESC(3, 0),                   // cable 3

        // inbound endpoint descriptors
        TUD_MIDI_DESC_EP(MIDI_ENDPOINT_COUNT, 64, CABLE_COUNT),      
        TUD_MIDI_JACKID_IN_EMB(1),                          // cable 1
        // TUD_MIDI_JACKID_IN_EMB(2),                       // cable 2
        // TUD_MIDI_JACKID_IN_EMB(3),                       // cable 3
        
        // outbound endpoint descriptors
        TUD_MIDI_DESC_EP(0x80 | MIDI_ENDPOINT_COUNT, 64, CABLE_COUNT),   
        TUD_MIDI_JACKID_OUT_EMB(1),
        // TUD_MIDI_JACKID_OUT_EMB(2),
        // TUD_MIDI_JACKID_OUT_EMB(3)
    };

    *itf += 2;  // +2 interface count(Audio Control, Midi Streaming)

    memcpy(dst, descriptor, TUSB_MIDI_DESCRIPTOR_LEN);
    return TUSB_MIDI_DESCRIPTOR_LEN;

}

USBPortConfig::USBPortConfig (
    int vendorId, int productId, int firmwareVersion, int usbPower, const char* productName,
    const char* productDescription, const char* manufacturerName, const char* serial ) {

        this->vendorId = vendorId;
        this->productId = productId;
        this->firmwareVersion = firmwareVersion;
        this->usbPower = usbPower;
        this->productName = productName;
        this->productDescription = productDescription;
        this->manufacturerName = manufacturerName;
        this->serial = serial;
};

void MidiDevice::install(const USBPortConfig& config) {

    // Change USB Device Descriptor Parameter
    USB.VID(config.vendorId); // vendor id
    USB.PID(config.productId);    
    USB.firmwareVersion(config.firmwareVersion);
    USB.usbPower(config.usbPower);
    USB.usbVersion(0x0200);
    USB.usbClass(TUSB_CLASS_AUDIO);
    USB.usbSubClass(0x00);
    USB.usbProtocol(0x00);
    USB.usbAttributes(0x80);
    
    if (config.serial != 0) USB.serialNumber(config.serial);
    if (config.productName != 0) USB.productName(config.productName);
    if (config.productDescription != 0) tinyusb_add_string_descriptor(config.productDescription);
    if (config.manufacturerName != 0) USB.manufacturerName(config.manufacturerName);

    tinyusb_enable_interface(USB_INTERFACE_MIDI, TUSB_MIDI_DESCRIPTOR_LEN, descriptorCallback);

    USB.onEvent(MidiDevice::usbCallback);        
    USB.begin();
}

bool MidiDevice::available() {
    return _available;
}

MidiDevice MIDI::MidiIO = MidiDevice();
