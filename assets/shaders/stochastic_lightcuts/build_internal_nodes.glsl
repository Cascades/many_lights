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

layout(binding = 0) buffer InputSSBO {
    PBTNode pbt[];
} input_ssbo;

layout(binding = 7) buffer BitonicSSBO {
    uint stage;
    uint pass_num;
    uint num_lights;
    uint max_lights;
} bitonic_vars;

void main()
{
    uint n = gl_GlobalInvocationID.x;
    uint node_d = uint(floor(log2(n + 1)));
    uint max_d = uint(floor(log2(input_ssbo.pbt.length())));
    uint target_d = max_d - node_d;

    uint left_leaf_node = uint(pow(2, target_d)) * n + (uint(pow(2, target_d)) - 1);
    uint right_leaf_node = uint(pow(2, target_d)) * n + (uint(pow(2, target_d + 1)) - 2);

    input_ssbo.pbt[n] = PBTNode(PBTBoundingBox(vec4(0.0), vec4(0.0)), vec4(0.0), ivec4(-1, 0, 0, 0));

    for (uint leaf_node_index = left_leaf_node; leaf_node_index <= right_leaf_node; ++leaf_node_index)
    {
        if (input_ssbo.pbt[leaf_node_index].total_intensity.x != 0.0)
        {
            input_ssbo.pbt[n].bb.min_bounds = min(input_ssbo.pbt[n].bb.min_bounds, input_ssbo.pbt[leaf_node_index].bb.min_bounds);
            input_ssbo.pbt[n].bb.max_bounds = max(input_ssbo.pbt[n].bb.max_bounds, input_ssbo.pbt[leaf_node_index].bb.max_bounds);
            input_ssbo.pbt[n].total_intensity.x += input_ssbo.pbt[leaf_node_index].total_intensity.x;
        }
    }

    input_ssbo.pbt[n].original_index = ivec4(n, left_leaf_node, right_leaf_node, 0);
    
}