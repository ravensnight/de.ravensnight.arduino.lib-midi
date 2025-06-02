#include <midi/SysexReceiver.h>
#include <async/LockGuard.h>

#include <Buffer.h>
#include <BufferInputStream.h>
#include <Logger.h>

using namespace ravensnight::midi;
using namespace ravensnight::logging;

SysexReceiver::SysexReceiver(size_t bufferSize, SysexHandler* handler) : _mutex("SysexReceiver") {
    _handler = handler;
    Logger::info("SysexReceiver buffer size is %d", bufferSize);
    _buffer = Buffer(bufferSize);
    _msgLen = 0;
}

SysexReceiver::~SysexReceiver() {
    _buffer.destroy();
}

void SysexReceiver::unsafeReset() {
    Logger::debug("SysexReceiver::reset");
    _state = SysexState::WAITING;
    _msgLen = 0;
}

void SysexReceiver::reset() {
    synchronized(_mutex);
    unsafeReset();
}

bool SysexReceiver::unsafeAppend(const MidiEvent& evt) {
    size_t avail = _buffer.length() - _msgLen;

    if (avail < evt.msgLength) {
        Logger::warn("SysexReceiver::handle - Buffer overrun. Reset.", evt.msgLength);
        reset();
        return false;
    }

    _msgLen += _buffer.set(_msgLen, evt.msg, evt.msgLength);
    // Logger::debug("SysexReceiver::apppend - new msg length = %d", _msgLen);
    return true;
}

void SysexReceiver::handle(const MidiEvent& evt) {
    synchronized(_mutex);

    bool trigger = false;
    if (_state == SysexState::WAITING) {
        if (evt.msgLength < 3) {
            Logger::warn("SysexReceiver::handle - Unexpected size %d (required 3). Return.", evt.msgLength);
            unsafeReset();
            return;    
        }

        if ((evt.type == CINType::SysexStart) && (evt.msg[0] == 0xF0)) {
            if (unsafeAppend(evt)) {
                _state = SysexState::READING;
            }
        }
    } else { // READING

        switch (evt.type) {
            case CINType::SysexStart:    // continue normally
            {
                if (evt.msgLength < 3) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 3). Return.", evt.msgLength);
                    reset();
                    return;    
                }
    
                unsafeAppend(evt);
                trigger = false;
                break;
            }

            case CINType::SysexEnd1:    // continue normally
            {
                if (evt.msgLength < 1) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 1). Return.", evt.msgLength);
                    reset();
                    return;    
                }
    
                trigger = unsafeAppend(evt);
                break;
            }

            case CINType::SysexEnd2:    // continue normally
            {
                if (evt.msgLength < 2) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 2). Return.", evt.msgLength);
                    reset();
                    return;    
                }
    
                trigger = unsafeAppend(evt);
                break;
            }

            case CINType::SysexEnd3:    // continue normally
            {
                if (evt.msgLength < 3) {
                    Logger::warn("SysexReceiver::handle - Unexpected size %d (required 3). Return.", evt.msgLength);
                    reset();
                    return;    
                }
    
                trigger = unsafeAppend(evt);
                break;
            }

            default:
                unsafeReset();
                break;
        }

        // trigger an action.
        if (trigger) {
            // Logger::dump("Midi message buffer is:", _buffer.bytes(), _msgLen, 0);    
            _handler->onSysEx(_buffer.bytes(), _msgLen);
            unsafeReset();
        }
    }   
}