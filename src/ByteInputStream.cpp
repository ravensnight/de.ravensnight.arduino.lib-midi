#include <midi/ByteInputStream.h>

ByteInputStream::ByteInputStream(const uint8_t* buffer, size_t len) {
    _buffer = buffer;
    _bufferLen = len;
}

ByteInputStream::~ByteInputStream() {
    _buffer = 0;
    _bufferLen = 0;
}

int16_t ByteInputStream::read() {
    if (_pos >= _bufferLen) return -1;
    int res = _buffer[_pos];
    _pos++;

    return res;
}

size_t ByteInputStream::read(uint8_t* buffer, size_t len) {
    if (_pos >= _bufferLen) return 0;

    size_t l = len;
    if ((_pos + l) >= _bufferLen) {
        l = _bufferLen - _pos;
    }

    memcpy(buffer, (_buffer + _pos), l);
    return l;
}

int16_t ByteInputStream::peek() {
    if (_pos >= _bufferLen) return -1;
    return _buffer[_pos];
}

size_t ByteInputStream::available() {
    if (_pos >= _bufferLen) return 0;
    return (_bufferLen - _pos);
}
