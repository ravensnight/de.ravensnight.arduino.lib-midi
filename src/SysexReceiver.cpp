#include <midi/SysexReceiver.h>
#include <midi/ByteInputStream.h>
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

bool SysexReceiver::append(const uint8_t* msg, size_t len) {

    if (_bufferPos + len > _bufferLen) {
        Logger::warn("SysexReceiver::handle - Buffer overrun. Reset.", len);
        reset();
        return false;
    }

    memcpy((_buffer + _bufferPos), msg, len);
    _bufferPos += len;

    return true;
}

void SysexReceiver::handle(CINType type, const uint8_t* msg, size_t len) {

    bool trigger = false;

    if (_state == SysexState::WAITING) {
        if (len < 3) {
            Logger::warn("SysexReceiver::handle - Unexpected size %d (required 3). Return.", len);
            reset();
            return;    
        }

        if ((type == CINType::SysexStart) && (msg[0] == 0xF0)) {
            if (append(msg, 3)) {
                _state = SysexState::READING;
            }
        }
    } else { // READING

        switch (type) {
            case CINType::SysexStart:    // continue normally
            {
                if (len < 3) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 3). Return.", len);
                    reset();
                    return;    
                }
    
                append(msg, 3);
                trigger = false;
                break;
            }

            case CINType::SysexEnd1:    // continue normally
            {
                if (len < 1) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 1). Return.", len);
                    reset();
                    return;    
                }
    
                trigger = append(msg, 1);
                break;
            }

            case CINType::SysexEnd2:    // continue normally
            {
                if (len < 2) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 2). Return.", len);
                    reset();
                    return;    
                }
    
                trigger = append(msg, 2);
                break;
            }

            case CINType::SysexEnd3:    // continue normally
            {
                if (len < 3) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 3). Return.", len);
                    reset();
                    return;    
                }
    
                trigger = append(msg, 3);
                break;
            }

            default:
                reset();
                break;
        }

        // trigger an action.
        if (trigger) {
            Logger::dump("Midi message buffer is:", _buffer, _bufferPos, 0);

            ByteInputStream* inputStream = new ByteInputStream(_buffer, _bufferPos);                           
            _handler->onSysEx(inputStream);
            delete inputStream;
            reset();
        }
    }   
}