
#include <midi/RolandSysexTypes.h>
#include <midi/RolandSysexHdr.h>
#include <midi/RolandSysexHandler.h>

#include <BufferInputStream.h>
#include <BufferOutputStream.h>

#include <Masquerade.h>
#include <Base128.h>
#include <Logger.h>

using namespace ravensnight::midi;
using namespace ravensnight::logging;

RolandSysexHandler::RolandSysexHandler(RolandSysexCallback* cb, MidiTransmitter* writer) {
    _cb = cb;
    _out = writer;
    _conv[Encoding::masquerade] = new Masquerade();
    _conv[Encoding::base128] = new Base128();
}

RolandSysexHandler::~RolandSysexHandler() {
    _cb = 0;
    _out = 0;

    delete _conv[Encoding::masquerade];
    delete _conv[Encoding::base128];
}

bool RolandSysexHandler::canHandle(uint8_t manufacturer) {
    return (manufacturer == ROLAND_MANUFACTURER_CODE);
}

void RolandSysexHandler::onSysEx(const uint8_t* buffer, size_t len) {
    const std::lock_guard<std::mutex> lock(_mutex);

    RolandSysexHdr hdr = {};
    BufferInputStream is(buffer, len);

    Logger::debug("Roland:Sysex - Read Header.");

    if (len < ROLAND_SYSEX_HDR_SIZE) {
        Logger::warn("Roland:Sysex - Header could not be read (received %d bytes). Stop.", len);
        return;
    }

    is >> hdr;

    Logger::debug("Roland:Sysex - Check device-ID.");
    uint8_t devId = _cb->getDeviceID();
    if (hdr.device != devId) {
        Logger::warn("Roland:Sysex - Device-ID does not match. Expected: %x, Received: %x", devId, hdr.device);
        return;
    }

    Logger::debug("Roland:Sysex - Check model-ID.");
    uint8_t modId = _cb->getModelID();
    if (hdr.model != modId) {
        Logger::warn("Roland:Sysex - Model-ID does not match. Expected: %x, Received: %x", modId, hdr.model);
        return;
    }

    RolandSysexAddr addr(hdr.addr);

    /*
    * Prozess the messages
    */
    if (hdr.cmd == ROLAND_SYSEX_CMD_READ) {
        Logger::debug("Roland:Sysex - Hande read command. Address: %06x", addr.get());
        handleCmdRead(addr, is);
    }
    else if (hdr.cmd == ROLAND_SYSEX_CMD_WRITE) {
        Logger::debug("Roland:Sysex - Hande write command. Address: %06x", addr.get());
        handleCmdWrite(addr, is);
    }
    else {
        Logger::warn("Roland:Sysex - invalid command received: %x", hdr.cmd);
    }
}

int RolandSysexHandler::handleCmdRead(RolandSysexAddr& addr, BufferInputStream& inputStream) {
    size_t midiByteLen = 0, encodedSize = 0;

    RecordInfo recordInfo = {};
    AddressInfo addressInfo = {};

    Logger::debug("Roland:Sysex:Read - Parse pull request.");
    
    // Read Size and Checksum ( 3 + 1)
    uint8_t payload[4];     
    if (inputStream.readBytes(payload, 4) < 4) {
        Logger::warn("Roland:Sysex:Read - Did not receive payload 4 bytes. Stop here.");
        return -1;
    }        

    if (!_cb->getAddressInfo(addr, addressInfo)) {
        Logger::warn("Roland:Sysex:Read - Requested address is invalid: %03x", addr.get());
        return -1;
    }

    // check checksum    
    Buffer payloadBuffer(payload, 0, 3);
    RolandSysexChecksum checksum2;
    checksum2 << addr;
    checksum2.add(payload, 3);

    if (payload[3] != checksum2.value()) {
        Logger::warn("Roland:Sysex:Read - Calculated checksum does not match received: %x, calculated: %x", payload[3], checksum2.value());            
        return -1;
    }

    // get record size
    midiByteLen = (__lsb(payload[0]) << 14) | (__lsb(payload[1]) << 7) | __lsb(payload[2]);
    Logger::debug("Roland:Sysex:Read - Midi size requested %d.", midiByteLen);
    
    // ------------------------------------------------------------------------
    // Loop all records
    // ------------------------------------------------------------------------

    Converter* conv = _conv[addressInfo.recordEncoding];

    Logger::debug("Loop records.");
    for (int rec = 0; rec < addressInfo.recordCount; rec++) {

        if (!_cb->getRecordInfo(addr, rec, recordInfo)) {
            Logger::warn("Invalid address / record num: %x > %x", addr.get(), rec);
            continue;
        }

        Logger::debug("Read record at addr: %06x, size=%d", recordInfo.addr.get(), recordInfo.size);

        encodedSize = conv->getEncodedSize(recordInfo.size);
        if (midiByteLen != encodedSize) {
            Logger::warn("Size requested %d does not match record size %d at %03x (record:%d)", midiByteLen, encodedSize, addr.get(), rec);
            continue;
        }

        // read data from model
        BufferOutputStream recordBuffer(recordInfo.size);
        _cb->readFromModel(recordInfo.addr, recordBuffer); 

        Logger::dump("Roland:Sysex:Read - Received record:", recordBuffer.buffer().bytes(), recordBuffer.buffer().length(), 0);        

        // create midi buffer
        BufferOutputStream midiOut(ROLAND_SYSEX_HDR_SIZE + encodedSize + 1);

        // copy full header
        midiOut << _cb->getDeviceID();
        midiOut << _cb->getModelID();
        midiOut << (uint8_t)ROLAND_SYSEX_CMD_WRITE;
        midiOut << recordInfo.addr;

        // convert record data to midi buffer
        Logger::debug("Roland:Sysex:Read - Encode record: %d > %d", recordBuffer.buffer().length(), encodedSize);        
        conv->encode(midiOut, recordBuffer.buffer().bytes(), recordBuffer.buffer().length());

        // create checksum
        checksum2.reset();
        checksum2.add(midiOut.buffer().bytesAt(3), 3 + encodedSize);  // skip deviceID, modelID, command; use address bytes (3) + encodedSize

        // add checksum to buffer
        midiOut << checksum2;

        Logger::debug("Roland:Sysex:Read - Send sysex checksum:%x len:%d.", checksum2.value(), midiOut.buffer().length());        
        _out->sendSysEx(ROLAND_SYSEX_MAN_CODE, midiOut.buffer());
    }

    return midiByteLen;
}

int RolandSysexHandler::handleCmdWrite(RolandSysexAddr& addr, BufferInputStream& inputStream) {
    Logger::debug("Roland:Sysex - Parse write request.");

    size_t len = 0;
    RecordInfo recordInfo = {};
    AddressInfo addressInfo = {};
    RolandSysexChecksum checksum;

    if (!_cb->getAddressInfo(addr, addressInfo)) {
        Logger::warn("Invalid address for write command: %x. Stop.", addr.get());
        return -1;
    }

    if (!_cb->getRecordInfo(addr, 0, recordInfo)) {
        Logger::warn("Record Info was not provided for address: %x. Stop.", addr.get());
        return -1;
    }

    Converter* conv = _conv[addressInfo.recordEncoding];

    size_t midiBufferSize = conv->getEncodedSize(recordInfo.size);
    size_t bufsize = midiBufferSize + 1;        // encoded stream size + checksum byte
    
    Buffer midiPayload(bufsize);    // buffer is size + 1 byte for checksum
    inputStream >> midiPayload;     // read the payload

    // Logger::dump("Midi payload received: ", midiPayload.bytes(), midiPayload.length(), 0);
    checksum << addr;

    Logger::dump("Add payload to checksum: ", midiPayload.bytes(), midiBufferSize, 0);
    checksum.add(midiPayload.bytes(), midiBufferSize);

    if (checksum.value() != midiPayload.bytes()[midiBufferSize]) {
        Logger::warn("Roland:Sysex:Write - Calculated checksum does not match received: %x, calculated: %x", midiPayload.bytes()[midiBufferSize], checksum.value());
        midiPayload.destroy();
        return -1;
    }

    // convert to model data.
    BufferOutputStream decodedBuffer(recordInfo.size);

    Logger::debug("Roland:Sysex:Write - decode payload from %d to %d", midiBufferSize, recordInfo.size);
    conv->decode(decodedBuffer, midiPayload.bytes(), midiBufferSize);

    Logger::debug("Roland:Sysex:Write - write decoded buffer to model.");
    BufferInputStream is(decodedBuffer.buffer());
    _cb->writeToModel(addr, is);

    Logger::debug("Roland:Sysex:Write - cleanup.");
    return recordInfo.size;
}
