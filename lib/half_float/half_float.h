#pragma once

#include "inttypes.h"

// https://stackoverflow.com/a/60047308

typedef unsigned short ushort;
typedef unsigned char ubyte;
typedef unsigned int uint;

uint as_uint(const float x);
float as_float(const uint x);

float fp16_to_fp32(const ushort x);
ushort fp32_to_fp16(const float x);

float fp8_to_fp32(const ubyte x, int exponent_bits = 4, int mantissa_bits = 3);
