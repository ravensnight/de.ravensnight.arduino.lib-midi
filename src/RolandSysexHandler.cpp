
#include <midi/RolandSysexTypes.h>
#include <midi/RolandSysexHandler.h>

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

void RolandSysexHandler::checksumAdd(int& checksum, uint8_t val) {
    checksum += val;
    if (checksum > 127) {
        checksum -= 128;
    }
}

void RolandSysexHandler::onSysEx(ByteInputStream* inputStream) {
    
    uint8_t buffer[6];
    uint8_t len;

    uint8_t start[2];
    len = inputStream->read(start, 2);
    if (len < 2) {
        Logger::error("Input stream did not provide the first two bytes of sysex stream.");
        return;
    }

    // check for roland message format at position 2 
    if (start[1] != 0x41) {
        Logger::debug("Manufacturer ID is not of type Roland: 0x%x. Skip whole SysEx message", start[1]);
        return;
    }

    if (handleSysEx(inputStream) < 0) {
        Logger::error("Failed to parse Roland SysEx message.");
    }
}

uint8_t RolandSysexHandler::checksum(RolandAddr& addr, uint8_t* bytes, uint16_t len) {
    int res = 0;

    checksumAdd(res, addr.hsb);    
    checksumAdd(res, addr.msb);    
    checksumAdd(res, addr.lsb);    

    for (int i = 0; i < len; i++) {
        checksumAdd(res, bytes[i]);    
    }

    res = 128 - res;
    return res == 128 ? 0 : res;
}

int RolandSysexHandler::handleCmdRead(RolandSysexHdr& hdr, ByteInputStream* inputStream) {
    size_t midiByteLen = 0, encodedSize = 0;

    RecordInfo recordInfo = {};
    AddressInfo addressInfo = {};

    uint8_t checksum1, checksum2;

    Logger::debug("Roland:Sysex:Read - Parse pull request.");

    uint8_t payload[4];     // 3 bytes for expected size + 1 byte checksum        
    if (inputStream->read(payload, 4) < 4) {
        Logger::warn("Roland:Sysex:Read - Did not receive payload 4 bytes. Stop here.");
        return -1;
    }        

    if (!_cb->getAddressInfo(hdr.addr, addressInfo)) {
        Logger::warn("Roland:Sysex:Read - Requested address is invalid: %x:%x:%x", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb);
        return -1;
    }

    // check checksum
    checksum1 = payload[3];
    checksum2 = RolandSysexHandler::checksum(hdr.addr, payload, 3);    // calculate checksum from hdr + length (3 bytes from buffer)  

    if (checksum1 != checksum2) {
        Logger::warn("Roland:Sysex:Read - Calculated checksum does not match received: %x, calculated: %x", checksum1, checksum2);            
        return -1;
    }

    // get record size
    midiByteLen = (__lsb(payload[0]) << 14) | (__lsb(payload[1]) << 7) | __lsb(payload[2]);
    Logger::debug("Roland:Sysex:Read - Midi size requested %d.", midiByteLen);

    hdr.cmd = ROLAND_SYSEX_CMD_WRITE; // Set the response command to SET

    // ------------------------------------------------------------------------
    // Loop all records
    // ------------------------------------------------------------------------

    Converter* conv = _conv[addressInfo.recordEncoding];

    for (int rec = 0; rec < addressInfo.recordCount; rec++) {

        if (!_cb->getRecordInfo(hdr.addr, rec, recordInfo)) {
            Logger::warn("Invalid address / record num: %x:%x:%x > %x", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb, rec);
            continue;
        }

        encodedSize = conv->getEncodedSize(recordInfo.size);
        if (midiByteLen != encodedSize) {
            Logger::warn("Size requested %d does not match record size %d at %x:%x:%x (record:%d)", midiByteLen, encodedSize, hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb, rec);
            continue;
        }


        // read data from model
        uint8_t recordBuffer[recordInfo.size];
        _cb->readFromModel(recordInfo.addr, recordBuffer, recordInfo.size); 

        // create midi buffer
        size_t midiBufferLen = ROLAND_SYSEX_HDR_SIZE + encodedSize + 1;
        uint8_t midiBuffer[midiBufferLen];

        // copy header without address to buffer
        memcpy(midiBuffer, &hdr, ROLAND_SYSEX_HDR_SIZE - 3);    

        // copy address to buffer
        memcpy(midiBuffer + (ROLAND_SYSEX_HDR_SIZE - 3), &(recordInfo.addr), 3);    

        // convert record data to midi buffer
        conv->encode(midiBuffer + ROLAND_SYSEX_HDR_SIZE, encodedSize, recordBuffer, recordInfo.size);

        // create checksum and add to buffer
        checksum2 = RolandSysexHandler::checksum(recordInfo.addr, (midiBuffer + ROLAND_SYSEX_HDR_SIZE), encodedSize);
        midiBuffer[ROLAND_SYSEX_HDR_SIZE + encodedSize] = checksum2;
        
        Logger::debug("Roland:Sysex:Read - Send sysex checksum:%x len:%d.", checksum, midiBufferLen);
        _out->sendSysEx(ROLAND_SYSEX_MAN_CODE, midiBuffer, midiBufferLen);
    }

    return midiByteLen;
}

int RolandSysexHandler::handleCmdWrite(RolandSysexHdr& hdr, ByteInputStream* inputStream) {
    Logger::debug("Roland:Sysex - Parse set request.");

    uint8_t checksum;
    size_t len = 0;
    RecordInfo recordInfo = {};
    AddressInfo addressInfo = {};

    if (!_cb->getAddressInfo(hdr.addr, addressInfo)) {
        Logger::warn("Invalid address for write command: %x:%x:%x. Stop.", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb);
        return -1;
    }

    if (!_cb->getRecordInfo(hdr.addr, 0, recordInfo)) {
        Logger::warn("Record Info was not provided for address: %x:%x:%x. Stop.", hdr.addr.hsb, hdr.addr.msb, hdr.addr.lsb);
        return -1;
    }

    Converter* conv = _conv[addressInfo.recordEncoding];

    size_t midiBufferSize = conv->getEncodedSize(recordInfo.size);
    size_t bufsize = midiBufferSize + 1;        // encoded stream size + checksum byte
    uint8_t midiPayload[bufsize];               // buffer is size + 1 byte for checksum
    uint8_t decodedBuffer[recordInfo.size];     // model buffer is record size

    len = inputStream->read(midiPayload, bufsize);   // read buffer incl. checksum
    if (len < bufsize) {
        Logger::warn("Could not read the sysex buffer of size %d. Received %d instead. Stop.", bufsize, len);
        return -1;
    }

    checksum = RolandSysexHandler::checksum(hdr.addr, midiPayload, midiBufferSize);
    if (checksum != midiPayload[midiBufferSize]) {
        Logger::warn("Roland:Sysex:Write - Calculated checksum does not match received: %x, calculated: %x", midiPayload[midiBufferSize], checksum);
        return -1;
    }

    // convert to model data.
    conv->decode(decodedBuffer, recordInfo.size, midiPayload, midiBufferSize);
    _cb->writeToModel(hdr.addr, decodedBuffer, recordInfo.size);

    return recordInfo.size;
}

int RolandSysexHandler::handleSysEx(ByteInputStream* inputStream) {

    RolandSysexHdr hdr = {};
    RecordInfo recordInfo = {};
    int recordCount;

    int checksum1 = 0, checksum2 = 0;
    size_t len, bufsize;

    Logger::debug("Roland:Sysex - Read Header.");
    len = inputStream->read(__buffer(&hdr), ROLAND_SYSEX_HDR_SIZE);

    if (len < ROLAND_SYSEX_HDR_SIZE) {
        Logger::warn("Roland:Sysex - Header could not be read (received %d bytes). Stop.", len);
        Logger::dump("Roland header received", __buffer(&hdr), len, 0);
        return -1;
    }

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

    if (hdr.cmd == ROLAND_SYSEX_CMD_READ) {
        return handleCmdRead(hdr, inputStream);
    }
    else if (hdr.cmd == ROLAND_SYSEX_CMD_WRITE) {
        return handleCmdWrite(hdr, inputStream);
    }
    else {
        Logger::warn("Roland:Sysex - invalid command received: %x", hdr.cmd);
        return -1;
    }
}