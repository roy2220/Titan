/*
 * Copyright (C) 2015 Roy O'Young <roy2220@outlook.com>.
 */


#pragma once


#include <stddef.h>
#include <stdint.h>


ptrdiff_t PackInteger8(char *, size_t, int8_t);
ptrdiff_t UnpackInteger8(const char *, size_t, int8_t *);
ptrdiff_t PackInteger16(char *, size_t, int16_t);
ptrdiff_t UnpackInteger16(const char *, size_t, int16_t *);
ptrdiff_t PackInteger32(char *, size_t, int32_t);
ptrdiff_t UnpackInteger32(const char *, size_t, int32_t *);
ptrdiff_t PackInteger64(char *, size_t, int64_t);
ptrdiff_t UnpackInteger64(const char *, size_t, int64_t *);
ptrdiff_t PackInteger(char *, size_t, intmax_t);
ptrdiff_t UnpackInteger(const char *, size_t, intmax_t *);
ptrdiff_t PackBytes(char *, size_t, const char *, size_t);
ptrdiff_t UnpackBytes(const char *, size_t, const char **, size_t *);
