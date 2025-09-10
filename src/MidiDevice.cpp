#include <esp32-hal-tinyusb.h>
#include <midi/LoggerConfig.h>
#include <midi/MidiDevice.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;
using namespace ravensnight::logging;

#ifndef MIDI_ENDPOINT_NUMBER 
    #define MIDI_ENDPOINT_NUMBER 1
#endif

ESP_EVENT_DEFINE_BASE(USB_MIDI_EVENTS);

MidiDevice::MidiDevice() {        
}

MidiDevice::~MidiDevice() {        
    _available = false;
    cableCount = 0;
    nameIndex = 0;
}

void MidiDevice::usbCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {

    if (event_base == ARDUINO_USB_EVENTS) {
        arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
        switch (event_id) {
            case ARDUINO_USB_STARTED_EVENT:
                _logger.info("USB PLUGGED");
                _available = true;
                break;

            case ARDUINO_USB_STOPPED_EVENT:
                _logger.info("USB UNPLUGGED");
                _available = false;
                break;

            case ARDUINO_USB_SUSPEND_EVENT:
                _logger.info("USB SUSPENDED: remote_wakeup_en: %d", data->suspend.remote_wakeup_en);
                _available = false;
                break;

            case ARDUINO_USB_RESUME_EVENT:
                _logger.info("USB RESUMED");
                _available = true;
                break;

            default:
                break;
        }
    } 
    else if (event_base == USB_MIDI_EVENTS) {
        _logger.debug("MIDI EVENT:  ID=%d, DATA=%d\r\n", event_id, (uint32_t)event_data);
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
    uint8_t desc1[] = { TUD_MIDI_DESC_EP(MIDI_ENDPOINT_NUMBER, 64, cableCount) };
    memcpy(dst + pos, desc1 , sizeof(desc1));
    pos += sizeof(desc1);

    for (uint8_t i = 0; i < cableCount; i++) {
        uint8_t jackIn[] = { TUD_MIDI_JACKID_IN_EMB(i + 1) };
        memcpy(dst + pos, jackIn, sizeof(jackIn));
        pos += sizeof(jackIn);
    }

    // add outbound endpoint descriptors
    uint8_t desc2[] = { TUD_MIDI_DESC_EP(0x80 | MIDI_ENDPOINT_NUMBER, 64, cableCount) };
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

int8_t MidiDevice::attach(const char* name, MidiReceiver* receiver) {
    if (cableCount < MAX_CABLE_COUNT) {        

        CableDef& def = cables[cableCount];

        strncpy(def.name, name, MAX_CABLE_NAMELEN);
        def.receiver = receiver;
        cableCount++;

        return (cableCount - 1);
    }

    return -1;
}

bool MidiDevice::setup(const USBConfig& config) {

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
        tinyusb_add_string_descriptor(cables[i].name);
    }

    tinyusb_enable_interface(USB_INTERFACE_MIDI, calculateDescriptorLength(), descriptorCallback);
    USB.onEvent(MidiDevice::usbCallback);        

    bool result = USB.begin();
    return result;
}

bool MidiDevice::available() {
    return _available;
}

uint8_t MidiDevice::getPacketLen(CINType tp) {
    switch (tp) {

        case CINType::SysexEnd1: 
        case CINType::SingleByte:
            return 1;

        case CINType::Common2Byte:
        case CINType::SysexEnd2:
        case CINType::ProgramChange:
        case CINType::ChannelPressure:
            return 2;

        case CINType::Common3Byte:
        case CINType::SysexStart:
        case CINType::SysexEnd3: 
        case CINType::NoteOff:
        case CINType::NoteOn:
        case CINType::PolyKey:
        case CINType::ControlChange:
        case CINType::ModulationWheel:
            return 3;

        default:
            // CINType::Misc
            // CINType::CableEvent
            return 0;            
    }
}

void MidiDevice::receive() {

    MidiEvent event = {
        .type = CINType::Reserved,
        .cable = 0,
        .msg = { 0 },
        .msgLength = 0
    };

    while (tud_midi_n_packet_read(0, _packet)) {

        uint8_t header = _packet[0];
        event.type = (CINType)(0x0F & header);
        if ((event.type == CINType::Reserved) || (event.type == CINType::CableEvent)) {
            // some invalid event. skip
            _logger.dump("Ignore packet.", _packet, 4, 0);
            continue;
        }

        event.cable = (header >> 4);
        _logger.trace("Received midi packet for pipe %d, cin: %02x", event.cable, event.type);
        if (event.cable < cableCount) {            
            MidiReceiver* r = cables[event.cable].receiver;

            event.msgLength = getPacketLen(event.type);
            memset(event.msg, 0, 3);
            memcpy(event.msg, _packet + 1, event.msgLength);

            _logger.trace("Received midi packet cable: %d, type: %d size: %d", event.cable, event.type, event.msgLength);
            _logger.dump("Midi packet msg: ", event.msg, event.msgLength, 0);
            r->handle(event);
        }
    }
}

size_t MidiDevice::publish(uint8_t cable, uint8_t* buffer, size_t size) {
    if (size == 0) return 0;

    if (!available()) {
        _logger.debug("MidiDevice::publish - Do not write to midi out, since USB is not available.");
        return 0;
    }

    int len = 0;
    uint8_t tries = 0;
    size_t sent = 0;
    do {        
        sent = tud_midi_n_stream_write(0, cable, buffer + len, size - len);

        if (sent == 0) {
            tries++;
            if (tries > MAX_TRIES_MIDIWRITE) {
                _logger.error("MidiDevice::publish - Only %d bytes of %d could be sent.");
                return len;
            }
        } else {
            len += sent;
        }

        _logger.debug("MidiDevice::publish - Sent %d bytes of %d", len, size);
    } while (len < size);

    return len;
}

MidiDevice MidiDevice::instance = MidiDevice();

// static initializers
bool MidiDevice::_available = false;
uint8_t MidiDevice::cableCount = 0;
uint8_t MidiDevice::nameIndex = 0;
CableDef MidiDevice::cables[MAX_CABLE_COUNT];
uint8_t MidiDevice::_packet[4] = { 0 };
ClassLogger MidiDevice::_logger(LC_MIDI_COMMON);