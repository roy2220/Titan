#include "Binary.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "Utility.h"


#define PACK_INTEGER(buffer, bufferSize, integer) \
    assert(buffer != NULL || bufferSize == 0);    \
    ptrdiff_t n = sizeof integer;                 \
                                                  \
    if (bufferSize < n) {                         \
        errno = ENOBUFS;                          \
        return bufferSize - n;                    \
    }                                             \
                                                  \
    buffer[n - 1] = integer;                      \
    ptrdiff_t i;                                  \
                                                  \
    for (i = n - 2; i >= 0; --i) {                \
        integer >>= CHAR_BIT;                     \
        buffer[i] = integer;                      \
    }                                             \
                                                  \
    return n

#define UNPACK_INTEGER(data, dataSize, integer)                   \
    assert(data != NULL || dataSize == 0);                        \
    assert(integer != NULL);                                      \
    ptrdiff_t n = sizeof *integer;                                \
                                                                  \
    if (dataSize < n) {                                           \
        errno = ENODATA;                                          \
        return dataSize - n;                                      \
    }                                                             \
                                                                  \
    *integer = (unsigned char)data[0];                            \
    ptrdiff_t i;                                                  \
                                                                  \
    for (i = 1; i < n; ++i) {                                     \
        *integer = *integer << CHAR_BIT | (unsigned char)data[i]; \
    }                                                             \
                                                                  \
    return n


ptrdiff_t
PackInteger8(char *buffer, size_t bufferSize, int8_t integer)
{
    PACK_INTEGER(buffer, bufferSize, integer);
}


ptrdiff_t
UnpackInteger8(const char *data, size_t dataSize, int8_t *integer)
{
    UNPACK_INTEGER(data, dataSize, integer);
}


ptrdiff_t
PackInteger16(char *buffer, size_t bufferSize, int16_t integer)
{
    PACK_INTEGER(buffer, bufferSize, integer);
}


ptrdiff_t
UnpackInteger16(const char *data, size_t dataSize, int16_t *integer)
{
    UNPACK_INTEGER(data, dataSize, integer);
}


ptrdiff_t
PackInteger32(char *buffer, size_t bufferSize, int32_t integer)
{
    PACK_INTEGER(buffer, bufferSize, integer);
}


ptrdiff_t
UnpackInteger32(const char *data, size_t dataSize, int32_t *integer)
{
    UNPACK_INTEGER(data, dataSize, integer);
}


ptrdiff_t
PackInteger64(char *buffer, size_t bufferSize, int64_t integer)
{
    PACK_INTEGER(buffer, bufferSize, integer);
}


ptrdiff_t
UnpackInteger64(const char *data, size_t dataSize, int64_t *integer)
{
    UNPACK_INTEGER(data, dataSize, integer);
}


ptrdiff_t
PackInteger(char *buffer, size_t bufferSize, intmax_t integer)
{
    assert(buffer != NULL || bufferSize == 0);
    intmax_t sign = integer >> sizeof integer * CHAR_BIT - 1;

    if (integer >> 6 == sign) {
        return PackInteger8(buffer, bufferSize, integer & UINTMAX_C(0x7F));
    }

    if (integer >> 13 == sign) {
        return PackInteger16(buffer, bufferSize, (integer | UINTMAX_C(0x8000)) & UINTMAX_C(0xBFFF));
    }

    if (integer >> 28 == sign) {
        return PackInteger32(buffer, bufferSize, (integer | UINTMAX_C(0xC0000000))
                                                 & UINTMAX_C(0xDFFFFFFF));
    }

    if (integer >> 59 == sign) {
        return PackInteger64(buffer, bufferSize, (integer | UINTMAX_C(0xE000000000000000))
                                                 & UINTMAX_C(0xEFFFFFFFFFFFFFFF));
    }

    if (bufferSize == 0) {
        errno = ENOBUFS;
        return -1;
    }

    buffer[0] = UCHAR_MAX;
    STATIC_ASSERT(sizeof(intmax_t) == sizeof(int64_t));
    ptrdiff_t result = PackInteger64(buffer + 1, bufferSize - 1, integer);

    if (result < 0) {
        return result;
    }

    return 1 + result;
}


ptrdiff_t
UnpackInteger(const char *data, size_t dataSize, intmax_t *integer)
{
    assert(data != NULL || dataSize == 0);
    assert(integer != NULL);

    if (dataSize == 0) {
        errno = ENODATA;
        return -1;
    }

    char data0 = data[0];

    if ((data0 & 1 << CHAR_BIT - 1) == 0) {
        int8_t temp;
        ptrdiff_t result = UnpackInteger8(data, dataSize, &temp);

        if (result < 0) {
            return result;
        }

        *integer = (int8_t)((uint8_t)temp << 1) >> 1;
        return result;
    }

    if ((data0 & 1 << CHAR_BIT - 2) == 0) {
        int16_t temp;
        ptrdiff_t result = UnpackInteger16(data, dataSize, &temp);

        if (result < 0) {
            return result;
        }

        *integer = (int16_t)((uint16_t)temp << 2) >> 2;
        return result;
    }

    if ((data0 & 1 << CHAR_BIT - 3) == 0) {
        int32_t temp;
        ptrdiff_t result = UnpackInteger32(data, dataSize, &temp);

        if (result < 0) {
            return result;
        }

        *integer = (int32_t)((uint32_t)temp << 3) >> 3;
        return result;
    }

    if ((data0 & 1 << CHAR_BIT - 4) == 0) {
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
PackBytes(char *buffer, size_t bufferSize, const char *bytes, size_t numberOfBytes)
{
    assert(buffer != NULL || bufferSize == 0);
    assert(bytes != NULL || numberOfBytes == 0);
    ptrdiff_t result = PackInteger(buffer, bufferSize, numberOfBytes);

    if (result < 0) {
        return result;
    }

    if (bufferSize < result + numberOfBytes) {
        errno = ENOBUFS;
        return bufferSize - result - numberOfBytes;
    }

    memcpy(buffer + result, bytes, numberOfBytes);
    return result + numberOfBytes;
}


ptrdiff_t
UnpackBytes(const char *data, size_t dataSize, const char **bytes, size_t *numberOfBytes)
{
    intmax_t temp;
    ptrdiff_t result = UnpackInteger(data, dataSize, &temp);

    if (result < 0) {
        return result;
    }

    *numberOfBytes = temp;

    if (dataSize < result + *numberOfBytes) {
        errno = ENODATA;
        return dataSize - result - *numberOfBytes;
    }

    *bytes = data + result;
    return result + *numberOfBytes;
}
