#pragma once
#include <algorithm>

namespace MortonCodeGenerator
{
    // interleaves uin32_t bits (morton code)
    inline uint32_t get_morton_bit_expansion(uint32_t v)
    {
        v = (v | v << 16) & 0x30000ff;
        v = (v | v << 8) & 0x300f00f;
        v = (v | v << 4) & 0x30c30c3;
        v = (v | v << 2) & 0x9249249;
        return v;
    }

    // converts floats in range [0, 1] into a single uint_32_t morton code
    // (10 bits per coord)
    inline uint32_t get_morton_code(float x, float y, float z)
    {
        x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
        y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
        z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
        uint32_t xx = get_morton_bit_expansion(static_cast<uint32_t>(x));
        uint32_t yy = get_morton_bit_expansion(static_cast<uint32_t>(y));
        uint32_t zz = get_morton_bit_expansion(static_cast<uint32_t>(z));
        return xx * 4 + yy * 2 + zz;
    }
};
