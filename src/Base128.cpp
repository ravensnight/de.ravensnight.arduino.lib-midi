#include <Base128.h>
#include <Logger.h>

using namespace LOGGING;

Base128::Base128() {
    // do nothing
}

size_t Base128::getDecodedSize(size_t encodedLen) {
    size_t res = encodedLen * 7 / 8;
    return res;
}

size_t Base128::decode(uint8_t* outBytes, size_t outBufferSize, const uint8_t* inBytes, size_t inSize) {
    size_t outSize = getDecodedSize(inSize);
    if (outSize > outBufferSize) return -1;

    uint8_t out = 0, in = 0, bit = 0;
    size_t outIndex = 0;

    for (size_t inIndex = 0; inIndex < inSize; inIndex++) {
        in = inBytes[inIndex];
        //Logger::debug("decode - source: %x", in);

        for (int i = 0; i < 7; i++) {
            out <<= 1;

            int match = (1 << (6 - i));
            if ((in & match) > 0) {
                out |= 0x1;
            }

            bit++;
            if (bit == 8) {
                //Logger::debug("decode - write out: %x", out);
                outBytes[outIndex] = out;
                bit = 0;
                out = 0;                    
                outIndex++;
            }
        }
    }

    return outIndex;
}

size_t Base128::getEncodedSize(size_t decodedLen) {
    size_t res = decodedLen * 8 / 7;
    if ((decodedLen * 8) % 7 > 0) {
        res++;
    }
    return res;
}

size_t Base128::encode(uint8_t* outBytes, size_t outBufferSize, const uint8_t* inBytes, size_t inSize) {
    size_t outSize = getEncodedSize(inSize);
    if (outSize > outBufferSize) return -1;

    uint8_t out = 0, bit = 0, in = 0; 
    size_t outIndex = 0;

    for (size_t inIndex = 0; inIndex < inSize; inIndex++) {
        in = inBytes[inIndex];
        //Logger::debug("encode - source: %x", in);

        for (int i = 0; i < 8; i++) {
            out <<= 1;
            int match = (1 << (7 - i));
            if ((in & match) > 0) {
                out |= 0x1;
            }
            bit++;
            if (bit == 7) {
                //Logger::debug("encode - write out: %x", out);
                outBytes[outIndex] = out;
                bit = 0;
                out = 0;   
                outIndex++;                 
            }
        }
    }

    if (bit > 0) {
        out <<= (7 - bit);
        //Logger::debug("encode - write out: %x", out);
        outBytes[outIndex] = out;
        outIndex++;                 
    }

    return outIndex;

}