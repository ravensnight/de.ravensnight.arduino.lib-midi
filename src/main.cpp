#include <Arduino.h>

#include <utils/Ref.hpp>
#include <midi/RolandSysexHandler.h>
#include <midi/RolandSysexCallback.h>
#include <midi/SysexReceiver.h>
#include <midi/VoiceReceiver.h>

using namespace ravensnight::midi;
using namespace ravensnight::utils;

class MySysexCallback : public RolandSysexCallback {
    public:
        uint8_t getDeviceID() { return 0x01; }
        uint8_t getModelID() { return 0x02; }

        bool getRecordInfo(const RolandSysexAddr& addr, int record, RecordInfo& info) {
            return false;
        }

        bool getAddressInfo(const RolandSysexAddr& src, AddressInfo& info) {
            return false;
        }

        Result writeToModel(const RolandSysexAddr& addr, Stream& istream) {
            return Result::reject;
        }

        Result readFromModel(const RolandSysexAddr& addr, Stream& istream) {
            return Result::reject;
        }
};

class MyVoiceCallback : public VoiceCallback {
    void onNoteOn(uint8_t chn, uint8_t pitch, uint8_t velocity) {};
    void onNoteOff(uint8_t chn, uint8_t pitch, uint8_t velocity) {};
    void onAftertouch(uint8_t chn, uint8_t pitch, uint8_t pressure) {};
    void onControlChange(uint8_t chn, uint8_t control, uint8_t value) {};
    void onProgramSelect(uint8_t chn, uint8_t prognum) {};
    void onChannelPressure(uint8_t chn, uint8_t pressure) {};
    void onModulationWheel(uint8_t chn, uint16_t pitchValue) {};
    void onSongPos(uint16_t pos) {};
    void onSongSel(uint8_t songNum) {};
    void onMidiStart() {};
    void onMidiStop() {};
    void onMidiContinue() {};    
};

Ref<MidiTransmitter> midiOut(new MidiTransmitter(0, 256));
Ref<RolandSysexCallback> callback(new MySysexCallback());
Ref<SysexHandler> roland(new RolandSysexHandler(1024, callback, midiOut));
Ref<SysexReceiver> sysexReceiver(new SysexReceiver(roland));

Ref<VoiceCallback> voiceCallback(new MyVoiceCallback());
Ref<VoiceReceiver> voiceReceiver(new VoiceReceiver(voiceCallback));

void setup() {
    MidiDevice::instance.attach("Voice", voiceReceiver.get());
    MidiDevice::instance.attach("Sysex", sysexReceiver.get());
}

void loop() {

}