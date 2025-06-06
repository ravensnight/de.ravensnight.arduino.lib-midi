#include <midi/SysexReceiver.h>
#include <Buffer.h>
#include <BufferInputStream.h>
#include <Logger.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;
using namespace ravensnight::logging;
using namespace ravensnight::async;

SysexReceiver::SysexReceiver(size_t bufferSize, SysexHandler* handler) : _mutex("SysexReceiver") {
    Logger::info("SysexReceiver buffer size is %d", bufferSize);
    _buffers[0] = Buffer(bufferSize);
    _buffers[1] = Buffer(bufferSize);

    _activeBuffer = 0;
    _handler = handler;
    _state = SysexState::open;
}

SysexReceiver::~SysexReceiver() {
    _buffers[0].destroy();
    _buffers[1].destroy();
    _activeBuffer = 0;
}

void SysexReceiver::unsafeReset() {
    _buffers[_activeBuffer].reset();
    _state = SysexState::open;
}

bool SysexReceiver::unsafeAppend(const uint8_t* buffer, uint8_t len) {
    
    if ((len > 0) && (_buffers[_activeBuffer].avail() < len)) {
        Logger::warn("SysexReceiver::handle - Buffer overrun. Reset.", len);
        return false;
    }

    _buffers[_activeBuffer].append(buffer, len);
    // Logger::debug("SysexReceiver::apppend - new msg length = %d", _buffers[_activeBuffer].length());
    return true;
}

bool SysexReceiver::accepted(CINType type) {
    switch (type) {
        case CINType::SysexStart:
        case CINType::SysexEnd1:
        case CINType::SysexEnd2:
        case CINType::SysexEnd3:
            return true;

        default:
            return false;
    }
}

void SysexReceiver::unsafeTrigger() {
    uint8_t current = _activeBuffer;
    _activeBuffer = (current == 0) ? 1 : 0;

    Logger::debug("Midi message received in buffer %d. Length = %d", current, _buffers[current].length());
    if (_buffers[current].length() < 80) {
        Logger::dump("Message is:", _buffers[current].bytes(), _buffers[current].length(), 0);
    }
    _handler->onSysEx(_buffers[current].bytes(), _buffers[current].length());
}

void SysexReceiver::handle(const MidiEvent& evt) {
    if (!accepted(evt.type) || _handler == 0) return;

    synchronized(_mutex);
    bool trigger = false;

    switch (evt.type) {
        case CINType::SysexStart:  {
            if (evt.msg[0] == 0xF0) {   // first byte
                unsafeReset();
                if (_handler->canHandle(evt.msg[1])) {  // check manufacturer
                    unsafeAppend(evt.msg + 2, 1);       // append last byte of 3 to buffer
                    _state = SysexState::reading;
                } else {
                    _state = SysexState::ignore;
                }
            } else {
                if (_state == SysexState::reading) {
                    unsafeAppend(evt.msg, evt.msgLength);
                }
            }
            break;
        }

        case CINType::SysexEnd1: 
        case CINType::SysexEnd2:
        case CINType::SysexEnd3:
        {
            uint8_t last = evt.msgLength - 1;
            if (_state == SysexState::reading) {
                if (evt.msg[last] == 0xF7) {
                    if (last > 0) {
                        unsafeAppend(evt.msg, last);
                    }

                    _state = SysexState::open;
                    trigger = true;
                }
            }
            break;
        }

        default:
            unsafeReset();
            break;
    }

    if (trigger) {
        unsafeTrigger();
    }
}