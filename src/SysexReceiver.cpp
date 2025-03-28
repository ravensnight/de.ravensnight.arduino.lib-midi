#include <midi/SysexReceiver.h>
#include <midi/ArrayInputStream.h>
#include <Logger.h>

using namespace MIDI;
using namespace LOGGING;

SysexReceiver::SysexReceiver(size_t bufferSize, SysexHandler* handler) {
    _handler = handler;
    _buffer = (uint8_t*)malloc(bufferSize);
    _bufferLen = bufferSize;
    _bufferPos = 0;
}

SysexReceiver::~SysexReceiver() {
    free(_buffer);
}

void SysexReceiver::reset() {
    _bufferPos = 0;
    _state = SysexState::WAITING;
}

void SysexReceiver::handle(CINType type, const uint8_t* buf, uint8_t len) {

    if (len > (_bufferLen - _bufferPos)) {
        Logger::instance.warn("Maximum sysex buffer length reached. Reset.");
        reset();
        return;
    }

    if (_state == SysexState::WAITING) {
        if ((type == CINType::SysexStart) && (buf[0] == 0xF0)) {
            memcpy(_buffer, buf, len);
            _state = SysexState::READING;
            _bufferPos = len;
        }
    } else { // READING

        switch (type) {
            case CINType::SysexStart:    // continue normally
                memcpy(_buffer, buf, len);
                _bufferPos += len;
                break;

            case CINType::SysexEnd1:    // continue normally
            case CINType::SysexEnd2:    // continue normally
            case CINType::SysexEnd3:    // continue normally
            {
                memcpy(_buffer, buf, len);
                _bufferPos += len;

                ArrayInputStream* inputStream = new ArrayInputStream(_buffer, _bufferPos);                                
                _handler->onSysEx(inputStream);
                delete inputStream;
                reset();
                break;
            }

            default:
                reset();
                break;
        }
    }   
}