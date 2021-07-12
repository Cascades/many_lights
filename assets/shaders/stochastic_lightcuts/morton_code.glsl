#version 460

layout(local_size_x = 1, local_size_y = 1) in;

layout(std430) buffer;

layout(binding = 0) buffer InputSSBO {
    uint stage;
    uint pass_num;
    uint original_indicies[];
} input_ssbo;

// interleaves uin32_t bits (morton code)
uint get_morton_bit_expansion(in uint v)
{
    v = (v | v << 16) & 0x30000ff;
    v = (v | v << 8) & 0x300f00f;
    v = (v | v << 4) & 0x30c30c3;
    v = (v | v << 2) & 0x9249249;
    return v;
}

// converts floats in range [0, 1] into a single uint_32_t morton code
// (10 bits per coord)
uint get_morton_code(in float x, in float y, in float z)
{
    x = min(max(x * 1024.0, 0.0), 1023.0);
    y = min(max(y * 1024.0, 0.0), 1023.0);
    z = min(max(z * 1024.0, 0.0), 1023.0);
    uint xx = get_morton_bit_expansion(uint(x));
    uint yy = get_morton_bit_expansion(uint(y));
    uint zz = get_morton_bit_expansion(uint(z));
    return xx * 4 + yy * 2 + zz;
}

void main()
{
        glm::vec<3, LightT> const& current_pos = lights.data()[light_index].position;
        morton_index_code.emplace_back(std::make_pair(light_index, MortonCodeGenerator::get_morton_code((current_pos.x - lights.x_bounds[0]) / x_dist,
            (current_pos.y - lights.y_bounds[0]) / y_dist,
            (current_pos.z - lights.z_bounds[0]) / z_dist)));
}