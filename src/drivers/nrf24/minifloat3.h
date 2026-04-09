#pragma once

#include <inttypes.h>

typedef unsigned char ubyte3;
typedef unsigned int uint3;

float as_float_3(const uint3 x)
{
    return *(float *)&x;
}

float minif_to_float3(const ubyte3 x, size_t exponent_bits = 4, size_t mantissa_bits = 3)
{
    const uint3 sign = (x >> (exponent_bits + mantissa_bits)) & 1;
    const uint3 exponent = (x >> mantissa_bits) & ((1 << exponent_bits) - 1);
    const uint3 mantissa = x & ((1 << mantissa_bits) - 1);

    uint3 out_i = sign << 31 | (exponent != 0) * ((((int32_t)exponent - 7 + 127) & 0xFF) << 23 | mantissa << 20); // sign | normalized
    // subnormal not implemented

    return as_float_3(out_i);
}
