
#include <midi/RolandSysexTypes.h>
#include <midi/RolandSysexHdr.h>
#include <midi/RolandSysexHandler.h>

#include <BufferInputStream.h>
#include <BufferOutputStream.h>

#include <Masquerade.h>
#include <Base128.h>
#include <Logger.h>

using namespace MIDI;
using namespace LOGGING;

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

void RolandSysexHandler::onSysEx(const uint8_t* buffer, size_t len) {
    const std::lock_guard<std::mutex> lock(_mutex);

    if (len < 2) {
        Logger::error("Input stream did not provide the first two bytes of sysex stream.");
        return;
    }

    // check for roland message format at position 2 
    if (buffer[1] != 0x41) {
        Logger::debug("Manufacturer ID is not of type Roland: 0x%x. Skip whole SysEx message", buffer[1]);
        return;
    }

    int res = handleSysEx(buffer + 2, len - 2);
    if (res < 0) {
        Logger::error("Failed to parse Roland SysEx message.");
    }
}

int RolandSysexHandler::handleCmdRead(RolandSysexAddr& addr, BufferInputStream& inputStream) {
    size_t midiByteLen = 0, encodedSize = 0;

    RecordInfo recordInfo = {};
    AddressInfo addressInfo = {};

    uint8_t checksum1;

    Logger::debug("Roland:Sysex:Read - Parse pull request.");
    
    uint8_t payload[4];     // 3 bytes for expected size + 1 byte checksum        
    if (inputStream.readBytes(payload, 4) < 4) {
        Logger::warn("Roland:Sysex:Read - Did not receive payload 4 bytes. Stop here.");
        return -1;
    }        

    if (!_cb->getAddressInfo(addr, addressInfo)) {
        Logger::warn("Roland:Sysex:Read - Requested address is invalid: %03x", addr.get());
        return -1;
    }

    // check checksum
    checksum1 = payload[3];
    
    Buffer payloadBuffer(payload, 3);
    RolandSysexChecksum checksum2;
    checksum2 << addr;
    checksum2 << payloadBuffer;

    if (checksum1 != checksum2.value()) {
        Logger::warn("Roland:Sysex:Read - Calculated checksum does not match received: %x, calculated: %x", checksum1, checksum2.value());            
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
        Buffer recordBuffer(recordInfo.size);
        _cb->readFromModel(recordInfo.addr, recordBuffer); 

        // create midi buffer
        Buffer midiBuffer(ROLAND_SYSEX_HDR_SIZE + encodedSize + 1);
        BufferOutputStream os(midiBuffer);

        // copy full header
        os << _cb->getDeviceID();
        os << _cb->getModelID();
        os << (uint8_t)ROLAND_SYSEX_CMD_WRITE;
        os << recordInfo.addr;

        // convert record data to midi buffer
        conv->encode(os, recordBuffer);

        // create checksum
        checksum2.reset();
        checksum2.add(midiBuffer.bytesAt(3), 3 + encodedSize);  // skip deviceID, modelID, command; use address bytes (3) + encodedSize

        // add checksum to buffer
        os << checksum2;

        Logger::debug("Roland:Sysex:Read - Send sysex checksum:%x len:%d.", checksum2.value(), midiBuffer.length());        
        _out->sendSysEx(ROLAND_SYSEX_MAN_CODE, midiBuffer);

        midiBuffer.destroy();
        recordBuffer.destroy();
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

    if (checksum.value() != midiPayload[midiBufferSize]) {
        Logger::warn("Roland:Sysex:Write - Calculated checksum does not match received: %x, calculated: %x", midiPayload[midiBufferSize], checksum.value());
        midiPayload.destroy();
        return -1;
    }

    // convert to model data.
    Buffer decodedBuffer(recordInfo.size);
    BufferOutputStream os(decodedBuffer);

    Logger::debug("Roland:Sysex:Write - decode payload from %d to %d", midiBufferSize, recordInfo.size);
    conv->decode(os, midiPayload);

    Logger::debug("Roland:Sysex:Write - write decoded buffer to model.");
    _cb->writeToModel(addr, decodedBuffer);

    Logger::debug("Roland:Sysex:Write - cleanup.", midiBufferSize, recordInfo.size);

    // free buffers
    midiPayload.destroy();
    decodedBuffer.destroy();

    Logger::debug("Roland:Sysex:Write - return.", midiBufferSize, recordInfo.size);
    return recordInfo.size;
}

int RolandSysexHandler::handleSysEx(const uint8_t* buffer, size_t len) {

    RolandSysexHdr hdr = {};
    BufferInputStream is(buffer, len);

    Logger::debug("Roland:Sysex - Read Header.");

    if (len < ROLAND_SYSEX_HDR_SIZE) {
        Logger::warn("Roland:Sysex - Header could not be read (received %d bytes). Stop.", len);
        return -1;
    }

    is >> hdr;

    Logger::debug("Roland:Sysex - Check device-ID.");
    uint8_t devId = _cb->getDeviceID();
    if (hdr.device != devId) {
        Logger::warn("Roland:Sysex - Device-ID does not match. Expected: %x, Received: %x", devId, hdr.device);
        return -1;
    }

    Logger::debug("Roland:Sysex - Check model-ID.");
    uint8_t modId = _cb->getModelID();
    if (hdr.model != modId) {
        Logger::warn("Roland:Sysex - Model-ID does not match. Expected: %x, Received: %x", modId, hdr.model);
        return -1;
    }

    RolandSysexAddr addr(hdr.addr);

    if (hdr.cmd == ROLAND_SYSEX_CMD_READ) {
        Logger::debug("Roland:Sysex - Hande read command. Address: %06x", addr.get());
        return handleCmdRead(addr, is);
    }
    else if (hdr.cmd == ROLAND_SYSEX_CMD_WRITE) {
        Logger::debug("Roland:Sysex - Hande write command. Address: %06x", addr.get());
        return handleCmdWrite(addr, is);
    }
    else {
        Logger::warn("Roland:Sysex - invalid command received: %x", hdr.cmd);
        return -1;
    }
}