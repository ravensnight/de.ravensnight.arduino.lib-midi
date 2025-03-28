#ifndef __ARRAY_INPUT_STREAM_H__
#define __ARRAY_INPUT_STREAM_H__

#include <Stream.h>

class ArrayInputStream : public Stream {

    private:

        size_t _pos = 0;
        size_t _bufferLen = 0;
        const uint8_t* _buffer;

    public:

        ArrayInputStream(const uint8_t* buffer, size_t len);

        int available();
        int read();
        int peek();

        size_t write(uint8_t b);
};

#endif // __ARRAY_INPUT_STREAM_H__