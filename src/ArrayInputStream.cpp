#include <midi/ArrayInputStream.h>

ArrayInputStream::ArrayInputStream(const uint8_t* buffer, size_t len) {
    _buffer = buffer;
    _bufferLen = len;
}

int ArrayInputStream::read() {
    if (_pos >= _bufferLen) return -1;
    int res = _buffer[_pos];
    _pos++;

    return res;
}

int ArrayInputStream::peek() {
    if (_pos >= _bufferLen) return -1;
    return _buffer[_pos];
}

int ArrayInputStream::available() {
    if (_pos >= _bufferLen) return 0;
    return (_bufferLen - _pos);
}

size_t ArrayInputStream::write(uint8_t b) {
    // do nothing
}