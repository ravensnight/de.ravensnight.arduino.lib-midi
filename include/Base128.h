#ifndef __BASE128_H__
#define __BASE128_H__

#include <Arduino.h>
#include <Converter.h>

class Base128 : public Converter {

    public:

    Base128();

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

#endif // __BASE128_H__