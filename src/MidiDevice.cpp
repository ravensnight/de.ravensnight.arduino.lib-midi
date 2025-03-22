#include <esp32-hal-tinyusb.h>
#include <Logger.h>

// #include <USB.h>
// #include <usb_descriptors.h>
#include <Adafruit_TinyUSB.h>
#include <midi/MidiDevice.h>

using namespace MIDI;
using namespace LOGGING;

USBPortConfig::USBPortConfig (
    int vendorId, int productId, int firmwareVersion, int usbVersion, int usbPower, const char* productName,
    const char* productDescription, const char* manufacturerName, const char* serial ) {

        this->vendorId = vendorId;
        this->productId = productId;
        this->firmwareVersion = firmwareVersion;
        this->usbVersion = usbVersion;
        this->usbPower = usbPower;
        this->productName = productName;
        this->productDescription = productDescription;
        this->manufacturerName = manufacturerName;
        this->serial = serial;
};

void MidiDevice::setup(const USBPortConfig& config) {

    // Change USB Device Descriptor Parameter
    /*
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
    if (config.manufacturerName != 0) USB.manufacturerName(config.manufacturerName);
    */

    TinyUSBDevice.setID(config.vendorId, config.productId);
    TinyUSBDevice.setVersion(0x0200);
    TinyUSBDevice.setProductDescriptor(config.productDescription);
    TinyUSBDevice.setManufacturerDescriptor(config.manufacturerName);
    TinyUSBDevice.setDeviceVersion(config.firmwareVersion);

    TinyUSBDevice.begin();
}

MidiStream* MidiDevice::addStream(const char* name) {
    MidiStream *str = 0;

    if (stream_count < MAX_MIDISTREAMS) {
        str = new MidiStream(name);
        stream_count++;
    }

    return str;
}

uint8_t MidiDevice::stream_count = 0;
