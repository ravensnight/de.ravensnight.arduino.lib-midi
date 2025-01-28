#ifndef __MIDI_READER_H__
#define __MIDI_READER_H__

#include <Arduino.h>
#include <Stream.h>
#include <midi/MidiDevice.h>

#define MIDI_INPUT_SYSEXEND 0xF7
#define MIDI_INPUT_EOF -1

namespace MIDI {

    class MidiReader;

    class MidiCallback {

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
            virtual void onSysEx(uint8_t manufacturer, MidiReader* reader) = 0;

    };

    enum class ReaderState : uint8_t {
        WaitStart   = 0x00,
        WaitPayload = 0x01
    };

    class MidiReader {

        private:
            MidiCallback* _midiCallback = 0;
            Stream* _stream;

            ReaderState _state = ReaderState::WaitStart;
            MessageType _command = MessageType::Reset;    // undefined command
            uint8_t _channel = 0x00;    // the channel extracted from first midi byte

            /**
             * Provide the message size for a given message type.
             */
            uint8_t payloadSize(MessageType type);

        public:

            MidiReader(Stream* stream, MidiCallback* callback);

            /**
             * Scan the input stream and trigger the callback.
             */
            void parse();

            /**
             * Read some sysex message part into given buffer.
             * As long as size matches the returned value, sysex message had not been finished yet.
             */
            uint16_t readSysEx(uint8_t buffer[], uint16_t size);

            /**
             * Read the sysex message to the end.
             */
            void skipSysEx();

    };

}

#endif // __MIDI_READER_H__