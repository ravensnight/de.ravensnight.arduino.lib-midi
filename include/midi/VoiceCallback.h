#ifndef __VOICE_CALLBACK_H__
#define __VOICE_CALLBACK_H__

#include <Arduino.h>

namespace ravensnight::midi {

    class VoiceCallback {

        public:

            virtual void onNoteOn(uint8_t chn, uint8_t pitch, uint8_t velocity) = 0;
            virtual void onNoteOff(uint8_t chn, uint8_t pitch, uint8_t velocity) = 0;
            virtual void onAftertouch(uint8_t chn, uint8_t pitch, uint8_t pressure) = 0;
            virtual void onControlChange(uint8_t chn, uint8_t control, uint8_t value) = 0;
            virtual void onProgramSelect(uint8_t chn, uint8_t prognum) = 0;
            virtual void onChannelPressure(uint8_t chn, uint8_t pressure) = 0;
            virtual void onModulationWheel(uint8_t chn, uint16_t pitchValue) = 0;
            virtual void onSongPos(uint16_t pos) = 0;
            virtual void onSongSel(uint8_t songNum) = 0;
            virtual void onMidiStart() = 0;
            virtual void onMidiStop() = 0;
            virtual void onMidiContinue() = 0;
    
    };
}



#endif // __VOICE_CALLBACK_H__