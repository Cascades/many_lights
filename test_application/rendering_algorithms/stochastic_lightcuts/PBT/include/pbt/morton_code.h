#pragma once
#include <algorithm>

namespace MortonCodeGenerator
{
    // interleaves uin32_t bits (morton code)
    inline uint32_t get_morton_bit_expansion(uint32_t v)
    {
        v = (v * 0x00010001u) & 0xFF0000FFu;
        v = (v * 0x00000101u) & 0x0F00F00Fu;
        v = (v * 0x00000011u) & 0xC30C30C3u;
        v = (v * 0x00000005u) & 0x49249249u;
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
