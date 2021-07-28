#version 460

layout(local_size_x = 1, local_size_y = 1) in;

layout(std430) buffer;

struct PBTBoundingBox
{
    vec4 min_bounds;
    vec4 max_bounds;
};

struct PBTNode
{
    PBTBoundingBox bb;
    vec4 total_intensity;
    ivec4 original_index;
};

struct Light
{
    vec4 position;
    vec4 color;
};

layout(binding = 0) buffer InputSSBO {
    PBTNode pbt[];
} input_ssbo;

layout(binding = 4) buffer Lights {
    Light lights[];
} lights;

layout(binding = 5) buffer VarsSSBO {
    vec4 max_bound;
    vec4 min_bound;
    uint num_leaves;
    uint num_lights;
} morton_vars;

layout(binding = 6) buffer MortonSSBO {
    uvec2 morton_index_code[];
} morton_ssbo;

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
    uint curr_index = gl_GlobalInvocationID.x;

    // get size of root bounding box
    float x_dist = morton_vars.max_bound.x - morton_vars.min_bound.x;
    float y_dist = morton_vars.max_bound.y - morton_vars.min_bound.y;
    float z_dist = morton_vars.max_bound.z - morton_vars.min_bound.z;

    //morton_ssbo.morton_index_code[curr_index * 2] = curr_index;
    if (lights.lights[curr_index].color.r + lights.lights[curr_index].color.g + lights.lights[curr_index].color.b != 0.0)
    {
        morton_ssbo.morton_index_code[curr_index].x = get_morton_code((lights.lights[curr_index].position.x - morton_vars.min_bound.x) / x_dist,
            (lights.lights[curr_index].position.y - morton_vars.min_bound.y) / y_dist,
            (lights.lights[curr_index].position.z - morton_vars.min_bound.z) / z_dist);
    }
    else
    {
        morton_ssbo.morton_index_code[curr_index].x = 0;
    }

    morton_ssbo.morton_index_code[curr_index].y = curr_index;

    input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + curr_index] = PBTNode(PBTBoundingBox(lights.lights[curr_index].position, lights.lights[curr_index].position), vec4(dot(vec3(1.0), lights.lights[curr_index].color.xyz)), ivec4(curr_index));

   // morton_ssbo.morton_index_code[curr_index] = curr_index;

    // fill leaves in order
    //for (size_t light_index = 0; light_index < lights.get_num_lights(); ++light_index)
    //{
    //    data[num_leaves - 1 + light_index].bb.min_bounds = lights.data()[morton_index_code[light_index].first].position;
    //    data[num_leaves - 1 + light_index].bb.max_bounds = lights.data()[morton_index_code[light_index].first].position;
    //    data[num_leaves - 1 + light_index].total_intensity.x = lights.data()[morton_index_code[light_index].first].color.r +
    //        lights.data()[morton_index_code[light_index].first].color.g +
    //        lights.data()[morton_index_code[light_index].first].color.b;
    //    data[num_leaves - 1 + light_index].original_index.r = morton_index_code[light_index].first;
    //}
}