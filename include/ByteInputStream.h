#ifndef __BYTE_INPUT_STREAM_H__
#define __BYTE_INPUT_STREAM_H__

#include <Arduino.h>

class ByteInputStream {

    private:

        size_t _pos = 0;
        size_t _bufferLen = 0;
        const uint8_t* _buffer;

    public:

        ByteInputStream(const uint8_t* buffer, size_t len);
        ~ByteInputStream();

        size_t available();
        int16_t peek();
        int16_t read();

        size_t read(uint8_t* buffer, size_t len);
};

#endif // __BYTE_INPUT_STREAM_H__