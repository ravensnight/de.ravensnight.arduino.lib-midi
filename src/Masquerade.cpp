#include <Masquerade.h>
#include <Logger.h>

using namespace LOGGING;

Masquerade::Masquerade() {
    _mask = 0xFF;
}

Masquerade::Masquerade(uint8_t mask) {
    _mask = mask;
}

size_t Masquerade::getEncodedSize(size_t sourceLen) {
    return sourceLen;
}

size_t Masquerade::getDecodedSize(size_t sourceLen) {
    return sourceLen;
}

size_t Masquerade::encode(uint8_t* outBytes, size_t outSize, const uint8_t* inBytes, size_t inSize) {
    if (inSize > outSize) {
        Logger::error("Masquerade::convert - Input size %d greater than output size %d. Skip conversion.", inSize, outSize);
        return -1;
    }

    for (size_t i = 0; i < inSize; i++) {
        outBytes[i] = inBytes[i] & _mask;
    };

    return inSize;
}

size_t Masquerade::decode(uint8_t* outBytes, size_t outSize, const uint8_t* inBytes, size_t inSize) {
    if (inSize > outSize) {
        Logger::error("Masquerade::convert - Input size %d greater than output size %d. Skip conversion.", inSize, outSize);
        return -1;
    }

    memcpy(outBytes, inBytes, inSize);
    return inSize;
}
