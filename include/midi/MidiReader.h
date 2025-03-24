#ifndef __MIDI_READER_H__
#define __MIDI_READER_H__

#include <Arduino.h>
#include <Stream.h>
#include <midi/MidiDevice.h>
#include <midi/VoiceCallback.h>
#include <midi/SysexHandler.h>

#define MIDI_INPUT_SYSEXEND 0xF7
#define MIDI_INPUT_EOF -1
#define run_if_notnull(ptr, fn) if (ptr != 0) ptr->fn;

namespace MIDI {

    enum class ReaderState : uint8_t {
        WaitStart   = 0x00,
        WaitPayload = 0x01
    };

    class MidiReader {

        private:
            Stream* _stream;

            VoiceCallback* _voiceCallback = 0;
            SysexHandler* _sysexHandler = 0;

            ReaderState _state = ReaderState::WaitStart;
            MessageType _command = MessageType::Reset;    // undefined command
            uint8_t _channel = 0x00;    // the channel extracted from first midi byte

            /**
             * Provide the message size for a given message type.
             */
            uint8_t payloadSize(MessageType type);

        public:

            MidiReader(Stream* stream);

            void enableVoice(VoiceCallback* cb);
            void enableSysex(SysexHandler* handler);

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