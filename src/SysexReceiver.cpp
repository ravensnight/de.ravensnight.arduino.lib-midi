#include <midi/LoggerConfig.h>
#include <midi/SysexReceiver.h>
#include <utils/Buffer.h>
#include <utils/BufferInputStream.h>
#include <async/LockGuard.h>

using namespace ravensnight::midi;
using namespace ravensnight::logging;

SysexReceiver::SysexReceiver(SysexHandler* handler) : _mutex("SysexReceiver") {
    _handler = handler;    
    assert(_handler != 0);
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
            _logger.trace("SysexReceiver::accepted - reject evt type: %d", type);
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
    acquirelock(_mutex);

    if (!accepted(evt.type)) {
        _logger.dump("SysexReceiver::handle - skipped unknown content:", evt.msg, evt.msgLength, 0);
        return;
    }

    switch (evt.type) {
        case CINType::SysexStart:  {
            if (evt.msg[0] == 0xF0) {   // first byte
                _logger.trace("SysexReceiver::handle[start]. size: %d", evt.msgLength);                    
                _handler->init();
                if (_handler->ready()) {
                    unsafeAppend(evt.msg + 1, evt.msgLength - 1);
                } else {
                    _logger.warn("SysexReceiver::handle[start]. not ready!");                    
                }
            } else {
                _logger.trace("SysexReceiver::handle[continue]. size: %d", evt.msgLength);                    
                if (_handler->ready()) {
                    unsafeAppend(evt.msg, evt.msgLength);
                } else {
                    _logger.warn("SysexReceiver::handle[continue]. not ready!");                    
                }
            }
            break;
        }

        case CINType::SysexEnd1: 
        case CINType::SysexEnd2:
        case CINType::SysexEnd3:
            _logger.trace("SysexReceiver::handle[end]. size: %d", evt.msgLength);
            if (_handler->ready()) {
                uint8_t last = evt.msgLength - 1;
                if (evt.msg[last] == 0xF7) {
                    if (last > 0) {
                        unsafeAppend(evt.msg, last);
                    }

                    _handler->commit();
                } else {
                    _logger.warn("SysexReceiver::handle[end]. Last element not 0xF7.");                    
                }
            } else {
                _logger.warn("SysexReceiver::handle[end]. not ready!");                    
            }
            break;

        default:
            break;
    }
}

ClassLogger SysexReceiver::_logger(LC_MIDI_SYSEX);