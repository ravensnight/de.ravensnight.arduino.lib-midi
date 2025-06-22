
#include <midi/RolandSysexTypes.h>
#include <midi/RolandSysexHdr.h>
#include <midi/RolandSysexHandler.h>

#include <BufferInputStream.h>
#include <BufferOutputStream.h>

#include <async/LockGuard.h>
#include <Masquerade.h>
#include <Base128.h>
#include <Logger.h>

using namespace ravensnight::midi;
using namespace ravensnight::logging;

RolandSysexHandler::RolandSysexHandler(size_t bufferSize, RolandSysexCallback* cb, MidiTransmitter* writer) :
    _reqBuffer(bufferSize)
{
    _cb = cb;
    _out = writer;
    _stage = Stage::undefined;
    _reqCommand = Command::undefined;
    _reqPayloadSize = 0;
    _conv[Encoding::masquerade] = new Masquerade();
    _conv[Encoding::base128] = new Base128();
}

RolandSysexHandler::~RolandSysexHandler() {
    _reqBuffer.destroy();
    _cb = 0;
    _out = 0;

    delete _conv[Encoding::masquerade];
    delete _conv[Encoding::base128];
}

void RolandSysexHandler::reset() {
    _reqBuffer.reset();
    _reqChecksum.reset();
    _reqCommand = Command::undefined;
    _reqPayloadSize = 0;
    _stage = Stage::undefined;
}

void RolandSysexHandler::init() {
    _reqBuffer.reset();
    _reqChecksum.reset();
    _reqCommand = Command::undefined;
    _reqPayloadSize = 0;
    _stage = Stage::manufacturer;
}

void RolandSysexHandler::append(uint8_t byte) {
    if (!ready()) {
        return;
    }

    switch (_stage) {
        case Stage::manufacturer:
            if (byte == ROLAND_SYSEX_MAN_CODE) {
                _stage = Stage::device;
            } else {
                Logger::warn("RolandSysexHander::append - invalid manufacturer: %02x", byte);
                reset();
            }
            return;

        case Stage::device:
            if (byte == _cb->getDeviceID()) {
                _stage = Stage::model;
            } else {
                Logger::warn("RolandSysexHander::append - invalid device id: %02x", byte);
                reset();
            }
            return;

        case Stage::model:
            if (byte == _cb->getModelID()) {
                _stage = Stage::command;
            } else {
                Logger::warn("RolandSysexHander::append - invalid model id: %02x", byte);
                reset();
            }
            return;

        case Stage::command:
            if ((byte == Command::read) || (byte == Command::write)) {
                _reqCommand = (Command)byte;
                _stage = Stage::address;
            } else {
                Logger::warn("RolandSysexHander::append - invalid command: %02x", byte);
                reset();
            }
            return;

        case Stage::address:
            _reqBuffer.append(byte);
            _reqChecksum.add(byte);

            if (_reqBuffer.length() == ROLAND_SYSEX_ADDR_SIZE) {
                BufferInputStream is(_reqBuffer);
                is >> _reqAddress;
                if (_cb->getAddressInfo(_reqAddress, _reqAddressInfo)) {
                    _stage = Stage::payload;

                    if (_reqCommand == Command::read) {
                        _reqPayloadSize = 3;
                    } else {
                        RecordInfo info;
                        _cb->getRecordInfo(_reqAddress, 0, info);

                        Converter* conv = _conv[ _reqAddressInfo.recordEncoding ];
                        _reqPayloadSize = conv->getEncodedSize(info.size);
                    }

                    _reqBuffer.reset();
                 } else {
                    Logger::warn("RolandSysexHander::append - address seemed to be invalid: %08x", _reqAddress.get());
                    reset();
                }
            }
            return;

        case Stage::payload:
            _reqBuffer.append(byte);
            _reqChecksum.add(byte);

            if (_reqBuffer.length() == _reqPayloadSize) {
                _stage = Stage::checksum;
            }
            return;

        case Stage::checksum:
            if (_reqChecksum.value() == byte) {
                _stage = Stage::complete;
            } else {
                Logger::warn("RolandSysexHander::append - checksum invalid. calculated: %d, received: %d", _reqChecksum.value(), byte);
                reset();
            }
            return;

        default: // undefined & complete
            Logger::warn("RolandSysexHander::append - invalid stage to receive data: %d", _stage);
            reset();
            break;
    }
}

void RolandSysexHandler::commit() {

    Result res = Result::error;
    if (_stage == Stage::complete) {
        BufferInputStream payload(_reqBuffer);

        switch (_reqCommand) {            
            case Command::read:
                handleCmdRead(_reqAddress, payload);
                break;

            case Command::write:
                handleCmdWrite(_reqAddress, payload);
                break;

            default:
                break;    
        }
    }

    reset();
}

bool RolandSysexHandler::ready() {
    return (_stage != Stage::undefined);
}

void RolandSysexHandler::sendReply(RolandSysexAddr& targetAddr, const uint8_t* decodedPayload, size_t len) {
        Logger::dump("Roland:Sysex:Read - Received record:", decodedPayload, len, 0);        

        // create midi buffer with correct size
        Converter* conv = _conv[_reqAddressInfo.recordEncoding];
        size_t encodedSize = conv->getEncodedSize(len);

        BufferOutputStream midiOut(ROLAND_SYSEX_HDR_SIZE + encodedSize + 1);

        // copy full header
        midiOut << _cb->getDeviceID();
        midiOut << _cb->getModelID();
        midiOut << (uint8_t)Command::write;
        midiOut << targetAddr;

        // convert record data to midi buffer
        Logger::debug("Roland:Sysex:Read - Encode record: %d > %d", len, encodedSize);        
        conv->encode(midiOut, decodedPayload, len);

        // create checksum
        RolandSysexChecksum checksum;
        checksum.add(midiOut.buffer().bytesAt(3), 3 + encodedSize);  // skip deviceID, modelID, command; use address bytes (3) + encodedSize

        // add checksum to buffer
        midiOut << checksum;

        Logger::debug("Roland:Sysex:Read - Send sysex checksum:%x len:%d.", checksum.value(), midiOut.buffer().length());        
        _out->sendSysEx(ROLAND_SYSEX_MAN_CODE, midiOut.buffer());

}

void RolandSysexHandler::handleCmdRead(RolandSysexAddr& addr, BufferInputStream& inputStream) {
    size_t midiByteLen = 0, encodedSize = 0;

    Logger::debug("RolandSysexHandler::handleCmdRead.");
    
    // Read Size 3
    uint8_t payload[3];     
    if (inputStream.readBytes(payload, 3) < 3) {
        Logger::warn("RolandSysexHandler::handleCmdRead - Did not receive size payload of 3 bytes. Stop here.");
    }        

    // get record size
    midiByteLen = (__lsb(payload[0]) << 14) | (__lsb(payload[1]) << 7) | __lsb(payload[2]);
    Logger::debug("RolandSysexHandler::handleCmdRead - Midi size requested %d.", midiByteLen);
    
    // ------------------------------------------------------------------------
    // Loop all records
    // ------------------------------------------------------------------------

    Converter* conv = _conv[_reqAddressInfo.recordEncoding];

    Logger::debug("Loop records.");
    RecordInfo recordInfo;
    for (int rec = 0; rec < _reqAddressInfo.recordCount; rec++) {

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
        BufferOutputStream recordStream(recordInfo.size);
        Result res = _cb->readFromModel(recordInfo.addr, recordStream); 

        if (res == Result::next || res == Result::success) {
            sendReply(recordInfo.addr, recordStream.buffer().bytes(), recordStream.buffer().length());
        } else {
            Logger::debug("Failed to acquire model data: %06x of size=%d", recordInfo.addr.get(), recordInfo.size);
        }
    }
}

void RolandSysexHandler::handleCmdWrite(RolandSysexAddr& addr, BufferInputStream& inputStream) {
    Logger::debug("RolandSysexHandler::handleCmdWrite.");

    Converter* conv = _conv[_reqAddressInfo.recordEncoding];
    
    Buffer midiPayload(_reqPayloadSize);    // buffer is size + 1 byte for checksum
    inputStream >> midiPayload;             // read the payload

    // convert to model data.
    size_t len = conv->getDecodedSize(_reqPayloadSize);
    BufferOutputStream decodedBuffer(len);

    Logger::debug("Roland:Sysex:Write - decode payload from %d to %d", _reqPayloadSize, len);
    conv->decode(decodedBuffer, midiPayload.bytes(), _reqPayloadSize);

    Logger::debug("Roland:Sysex:Write - write decoded buffer to model.");
    BufferInputStream is(decodedBuffer.buffer());
    Result res = _cb->writeToModel(addr, is);

    // send ack, if required for this address.
    if (_reqAddressInfo.replyAck) {

        BufferOutputStream ackStream(ACK_REPLY_SIZE);
        ackStream << addr;
        ackStream << (uint8_t)res;

        sendReply(_reqAddressInfo.replyAddr, ackStream.buffer().bytes(), ackStream.buffer().length());
    }

    Logger::debug("Roland:Sysex:Write - cleanup.");
}
