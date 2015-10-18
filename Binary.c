/*
 * Copyright (C) 2015 Roy O'Young <roy2220@outlook.com>.
 */


#include "Binary.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "Utility.h"


#define INTEGER_PACKER(n)                                               \
    ptrdiff_t                                                           \
    PackInteger##n(int##n##_t integer, char *buffer, size_t bufferSize) \
    {                                                                   \
        assert(buffer != NULL || bufferSize == 0);                      \
        assert(bufferSize <= PTRDIFF_MAX);                              \
                                                                        \
        if (bufferSize < sizeof integer) {                              \
            errno = ENOBUFS;                                            \
            return bufferSize - sizeof integer;                         \
        }                                                               \
                                                                        \
        buffer[sizeof integer - 1] = integer;                           \
        ptrdiff_t i;                                                    \
                                                                        \
        for (i = sizeof integer - 2; i >= 0; --i) {                     \
            integer >>= CHAR_BIT;                                       \
            buffer[i] = integer;                                        \
        }                                                               \
                                                                        \
        return sizeof integer;                                          \
    }

#define INTEGER_UNPACKER(n)                                                  \
    ptrdiff_t                                                                \
    UnpackInteger##n(const char *data, size_t dataSize, int##n##_t *integer) \
    {                                                                        \
        assert(data != NULL || dataSize == 0);                               \
        assert(dataSize <= PTRDIFF_MAX);                                     \
        assert(integer != NULL);                                             \
                                                                             \
        if (dataSize < sizeof *integer) {                                    \
            errno = ENODATA;                                                 \
            return dataSize - sizeof *integer;                               \
        }                                                                    \
                                                                             \
        *integer = (unsigned char)data[0];                                   \
        ptrdiff_t i;                                                         \
                                                                             \
        for (i = 1; i < (ptrdiff_t)sizeof *integer; ++i) {                   \
            *integer = *integer << CHAR_BIT | (unsigned char)data[i];        \
        }                                                                    \
                                                                             \
        return sizeof *integer;                                              \
    }


INTEGER_PACKER(8)


INTEGER_UNPACKER(8)


INTEGER_PACKER(16)


INTEGER_UNPACKER(16)


INTEGER_PACKER(32)


INTEGER_UNPACKER(32)


INTEGER_PACKER(64)


INTEGER_UNPACKER(64)


#undef INTEGER_PACKER
#undef INTEGER_UNPACKER


ptrdiff_t
PackVariableLengthInteger(intmax_t integer, char *buffer, size_t bufferSize)
{
    assert(buffer != NULL || bufferSize == 0);
    assert(bufferSize <= PTRDIFF_MAX);
    intmax_t sign = integer >> (sizeof integer * CHAR_BIT - 1);

    if (integer >> 6 == sign) {
        return PackInteger8(integer & UINTMAX_C(0x7F), buffer, bufferSize);
    }

    if (integer >> 13 == sign) {
        return PackInteger16((integer | UINTMAX_C(0x8000)) & UINTMAX_C(0xBFFF), buffer, bufferSize);
    }

    if (integer >> 28 == sign) {
        return PackInteger32((integer | UINTMAX_C(0xC0000000)) & UINTMAX_C(0xDFFFFFFF), buffer
                             , bufferSize);
    }

    if (integer >> 59 == sign) {
        return PackInteger64((integer | UINTMAX_C(0xE000000000000000))
                             & UINTMAX_C(0xEFFFFFFFFFFFFFFF), buffer, bufferSize);
    }

    if (bufferSize < 1) {
        errno = ENOBUFS;
        return -1;
    }

    buffer[0] = UCHAR_MAX;
    STATIC_ASSERT(sizeof(intmax_t) == sizeof(int64_t));
    ptrdiff_t result = PackInteger64(integer, buffer + 1, bufferSize - 1);

    if (result < 0) {
        return result;
    }

    return 1 + result;
}


ptrdiff_t
UnpackVariableLengthInteger(const char *data, size_t dataSize, intmax_t *integer)
{
    assert(data != NULL || dataSize == 0);
    assert(dataSize <= PTRDIFF_MAX);
    assert(integer != NULL);

    if (dataSize < 1) {
        errno = ENODATA;
        return -1;
    }

    char data0 = data[0];

    if ((data0 & 1 << (CHAR_BIT - 1)) == 0) {
        int8_t temp;
        ptrdiff_t result = UnpackInteger8(data, dataSize, &temp);

        if (result < 0) {
            return result;
        }

        *integer = (int8_t)((uint8_t)temp << 1) >> 1;
        return result;
    }

    if ((data0 & 1 << (CHAR_BIT - 2)) == 0) {
        int16_t temp;
        ptrdiff_t result = UnpackInteger16(data, dataSize, &temp);

        if (result < 0) {
            return result;
        }

        *integer = (int16_t)((uint16_t)temp << 2) >> 2;
        return result;
    }

    if ((data0 & 1 << (CHAR_BIT - 3)) == 0) {
        int32_t temp;
        ptrdiff_t result = UnpackInteger32(data, dataSize, &temp);

        if (result < 0) {
            return result;
        }

        *integer = (int32_t)((uint32_t)temp << 3) >> 3;
        return result;
    }

    if ((data0 & 1 << (CHAR_BIT - 4)) == 0) {
        int64_t temp;
        ptrdiff_t result = UnpackInteger64(data, dataSize, &temp);

        if (result < 0) {
            return result;
        }

        *integer = (int64_t)((uint64_t)temp << 4) >> 4;
        return result;
    }

    STATIC_ASSERT(sizeof(intmax_t) == sizeof(int64_t));
    ptrdiff_t result = UnpackInteger64(data + 1, dataSize - 1, integer);

    if (result < 0) {
        return result;
    }

    return 1 + result;
}


ptrdiff_t
PackBytes(const char *bytes, size_t numberOfBytes, char *buffer, size_t bufferSize)
{
    assert(bytes != NULL || numberOfBytes == 0);
    assert(numberOfBytes <= PTRDIFF_MAX);
    assert(buffer != NULL || bufferSize == 0);
    assert(bufferSize <= PTRDIFF_MAX);
    ptrdiff_t result = PackVariableLengthInteger(numberOfBytes, buffer, bufferSize);

    if (result < 0) {
        return result;
    }

    if (bufferSize - result < numberOfBytes) {
        errno = ENOBUFS;
        return bufferSize - result - numberOfBytes;
    }

    memcpy(buffer + result, bytes, numberOfBytes);
    return result + numberOfBytes;
}


ptrdiff_t
UnpackBytes(const char *data, size_t dataSize, const char **bytes, size_t *numberOfBytes)
{
    assert(data != NULL || dataSize == 0);
    assert(dataSize <= PTRDIFF_MAX);
    assert(bytes != NULL);
    assert(numberOfBytes != NULL);
    intmax_t temp;
    ptrdiff_t result = UnpackVariableLengthInteger(data, dataSize, &temp);

    if (result < 0) {
        return result;
    }

    *numberOfBytes = temp;

    if (*numberOfBytes > PTRDIFF_MAX) {
        errno = EINVAL;
        return -1;
    }

    if (dataSize - result < *numberOfBytes) {
        errno = ENODATA;
        return dataSize - result - *numberOfBytes;
    }

    *bytes = data + result;
    return result + *numberOfBytes;
}
