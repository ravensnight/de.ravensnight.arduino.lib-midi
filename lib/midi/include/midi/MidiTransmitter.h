#ifndef __MIDI_WRITER_H__
#define __MIDI_WRITER_H__

#include <Arduino.h>
#include <Stream.h>

#include <Logger.h>
#include <utils/Buffer.h>
#include <midi/MidiDevice.h>

using namespace ravensnight::utils;
namespace ravensnight::midi {

    class MidiTransmitter {

        private:
            static Logger _logger;

            uint8_t _cable;
            Buffer _outBuffer;

            // write binary data to midi out for the cable configured.
            size_t write(uint8_t* buffer, size_t size);

        public:

        // constructors
        MidiTransmitter(uint8_t cable, size_t outBufferSize);
        MidiTransmitter(MidiTransmitter& source);
        
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
         * Send a sysex message as raw bytes. 
         * Note: 
         * The given input data is expected to contain 7bit data up to number 127 (7F).
         * If it does not match it is being masqueraded.
         * 
         * @param manCode the manufacturer code as 3byte code: 
         *  - { 0x41, 0x00, 0x00 } for a dedicated single byte manufacturer id
         *  - { 0x00, 0x22, 0x23 } any other specific 3-byte id.
         * 
         * @param buffer holds the bytes to be converted and sent
         * @return the number of bytes sent.
         * 
         */
        size_t sendSysEx(SysexManCode& manCode, Buffer& buffer);

    };

}

#endif // __MIDI_WRITER_H__