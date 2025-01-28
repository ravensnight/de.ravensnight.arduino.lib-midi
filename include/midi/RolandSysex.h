#ifndef __ROLAND_SYSEX_H__
#define __ROLAND_SYSEX_H__

#include <Arduino.h>
#include <midi/MidiReader.h>
#include <midi/MidiWriter.h>

namespace MIDI {

    #define ROLAND_SYSEX_MAN_CODE   0x41
    #define ROLAND_SYSEX_HDR_SIZE   6
    #define ROLAND_SYSEX_CMD_READ   0x11
    #define ROLAND_SYSEX_CMD_WRITE  0x12


    typedef struct {        
        volatile uint8_t hsb;    // highest significant byte
        volatile uint8_t msb;    // middle/medium significant byte
        volatile uint8_t lsb;    // least significant byte

    } RolandAddr;

    /**
     * Defines the record info which is required to 
     * read or send some sysex message.
     */
    typedef struct {
        uint16_t index;     // the record number, this info belongs to.
        uint16_t size;      // the size of the record
        RolandAddr addr;    // the address of the record.
    } RecordInfo;

    typedef struct {
        volatile uint8_t device;
        volatile uint8_t model;
        volatile uint8_t cmd;
        
        RolandAddr addr;
    } RolandSysexHdr;

    class RolandSysexCallback {
        public:

            /**
             * Provide the device ID expected to be set in message.
             */
            virtual uint8_t getDeviceID() = 0;

            /**
             * Provide the model ID expected to be set in message.
             */
            virtual uint8_t getModelID() = 0;

            /**
             * Provide the number of bytes to be read or written in a single sysex record
             * If the address is invalid false is returned, otherwise true
             * @param addr the roland address
             * @param record the record number to get all record info for
             * @param info the info being provided by callback instance
             */        
            virtual bool getRecordInfo(const RolandAddr& addr, int record, RecordInfo& info) = 0;

            /**
             * Provide the number of records which shall be read or sent.
             * If the given address is invalid -1 shall be returned.
             */
            virtual int getRecordCount(const RolandAddr& src) = 0;

            /**
             * Apply the data which has been received from sysex
             */
            virtual void write(const RolandAddr& addr, const uint8_t buffer[], uint16_t size) = 0;

            /**
             * Read some data from addr and provide it to buffer.
             */
            virtual void read(const RolandAddr& addr, uint8_t buffer[], uint16_t size) = 0;

    };

    class RolandSysex {

        public:

            /**
             * Parse a message from Midi In-Stream and potentially respond to writer.
             * Returns the number of bytes read or sent as payload or -1, if address was invalid or message could not be parsed correctly.
             */
            static int readSysEx(MidiReader* reader, MidiWriter* writer, RolandSysexCallback* cb);

            /**
             * Calculate the checksum
             */
            static uint8_t checksum(RolandAddr& addr, uint8_t bytes[], uint16_t len);

        private:

            MidiReader* _reader;
            MidiWriter* _writer;
            RolandSysexCallback* _callback;

            static void checksumAdd(int& previous, uint8_t value);
    };
}

#endif // __ROLAND_SYSEX_H__