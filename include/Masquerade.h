#ifndef __BITMASK_H__
#define __BITMASK_H__

#include <Converter.h>

class Masquerade : public Converter {

    private:
        uint8_t _mask = 0xFF;

    public:

    Masquerade();
    Masquerade(uint8_t mask);

    /**
     * Enccode some given inBytes to outBytes.
     */
    size_t encode(uint8_t* outBytes, size_t outSize, const uint8_t* inBytes, size_t inSize);

    /**
     * Decode some given inBytes to outBytes.
     */
    size_t decode(uint8_t* outBytes, size_t outSize, const uint8_t* inBytes, size_t inSize);

    /**
     * Get the size of the encoded stream from given source
     */
    size_t getEncodedSize(size_t decodedLen);

    /**
     * Get the size of the decoded stream from given source
     */
    size_t getDecodedSize(size_t encodedLen);
};


#endif // __BITMASK_H__