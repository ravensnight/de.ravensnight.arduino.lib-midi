#ifndef __MIDI_WRITER_H__
#define __MIDI_WRITER_H__

#include <Arduino.h>
#include <Stream.h>
#include <midi/MidiDevice.h>

namespace MIDI {

    class MidiTransmitter {

        private:
            uint8_t _cable;

            // write binary data to midi out for the cable configured.
            size_t write(const uint8_t *buf, size_t size);

        public:

        // constructors
        MidiTransmitter(uint8_t cable);
        ~MidiTransmitter();

        // generic midi send function
        void send(MessageType msg, uint8_t channel, uint16_t value);
        void send(MessageType msg, uint8_t channel, uint8_t value1, uint8_t value2);

        // voice messages
        void sendNoteOn(uint8_t chn, uint8_t pitch, uint8_t velocity);
        void sendNoteOff(uint8_t chn, uint8_t pitch, uint8_t velocity);
        void sendAftertouch(uint8_t chn, uint8_t pitch, uint8_t pressure);
        void sendControlChange(uint8_t chn, uint8_t control, uint8_t value);
        void sendProgramSelect(uint8_t chn, uint8_t prognum);
        void sendChannelPressure(uint8_t chn, uint8_t pressure);
        void sendModulationWheel(uint8_t chn, int16_t pitchValue);

        // system messages
        void sendSongPos(int16_t pos);
        void sendSongSel(int8_t songnum);
        void sendMidiStart();
        void sendMidiStop();
        void sendMidiContinue();

        /**
         * Send a sysex message.
         * @param channel can be one of:
         *  - the predefined manufacturer ID or 
         *  - 0x00,  where buffer must contain first 2 bytes of a new-manufacturer ID
         *  - 0x7E for Non-Realtime Messages or 
         *  - 0x7F for Realtime messages
         *  - any other specific ID.
         * 
         * @param buffer holds the bytes to be sent
         * @param len informs about the number of bytes to read from buffer.
         */
        void sendSysEx(uint8_t channel, uint8_t* buffer, uint16_t len);
    };

}

#endif // __MIDI_WRITER_H__