#include <midi/SysexReceiver.h>
#include <Buffer.h>
#include <BufferInputStream.h>
#include <Logger.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;
using namespace ravensnight::logging;
using namespace ravensnight::async;

SysexReceiver::SysexReceiver(SysexHandler* handler) : _mutex("SysexReceiver") {
    _handler = handler;
}

SysexReceiver::~SysexReceiver() {
}

bool SysexReceiver::accepted(CINType type) {
    switch (type) {
        case CINType::SysexStart:
        case CINType::SysexEnd1:
        case CINType::SysexEnd2:
        case CINType::SysexEnd3:
            return true;

        default:
            // Logger::debug("SysexReceiver::accepted - reject evt type: %d", type);
            return false;
    }
}

void SysexReceiver::unsafeAppend(const uint8_t* buffer, uint8_t len) {
    if (len == 0) return;
    for (uint8_t i = 0; i < len; i++) {
        _handler->append(buffer[i]);
    }
}

void SysexReceiver::handle(const MidiEvent& evt) {
    if (!accepted(evt.type) || _handler == 0) return;

    synchronized(_mutex);
    // Logger::debug("SysexReceiver::handle - msg:[%02x, %02x, %02x] len:%d", evt.msg[0], evt.msg[1], evt.msg[2], evt.msgLength);

    switch (evt.type) {
        case CINType::SysexStart:  {
            // Logger::debug("SysexReceiver::handle - start/continue. size: %d", evt.msgLength);
            if (evt.msg[0] == 0xF0) {   // first byte
                _handler->init();
                if (_handler->ready()) {
                    unsafeAppend(evt.msg + 1, evt.msgLength - 1);
                }
            } else {
                if (_handler->ready()) {
                    unsafeAppend(evt.msg, evt.msgLength);
                }
            }
            break;
        }

        case CINType::SysexEnd1: 
        case CINType::SysexEnd2:
        case CINType::SysexEnd3:
            // RecordInfoLogger::debug("SysexReceiver::handle - end. size: %d", evt.msgLength);
            if (_handler->ready()) {
                uint8_t last = evt.msgLength - 1;
                if (evt.msg[last] == 0xF7) {
                    if (last > 0) {
                        unsafeAppend(evt.msg, last);
                    }

                    _handler->commit();
                }
            }
            break;

        default:
            break;
    }
}