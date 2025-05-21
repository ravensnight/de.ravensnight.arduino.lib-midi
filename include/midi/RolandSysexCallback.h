#ifndef __ROLAND_SYSEX_CALLBACK__
#define __ROLAND_SYSEX_CALLBACK__

#include <Arduino.h>
#include <midi/RolandSysexTypes.h>

namespace MIDI {

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
            virtual bool getRecordInfo(const RolandSysexAddr& addr, int record, RecordInfo& info) = 0;

            /**
             * Provide the number of records which shall be read or sent.
             * If the given address is invalid -1 shall be returned.
             */
            virtual bool getAddressInfo(const RolandSysexAddr& src, AddressInfo& info) = 0;

            /**
             * Apply the data which has been received from sysex
             */
            virtual void writeToModel(const RolandSysexAddr& addr, const uint8_t buffer[], uint16_t size) = 0;

            /**
             * Read some data from addr and provide it to buffer.
             */
            virtual void readFromModel(const RolandSysexAddr& addr, uint8_t buffer[], uint16_t size) = 0;

    };

}

#endif // __ROLAND_SYSEX_CALLBACK__