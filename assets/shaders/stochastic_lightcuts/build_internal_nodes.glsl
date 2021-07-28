#version 460

layout(local_size_x = 8, local_size_y = 8) in;

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
    uint level;
    uint level_size;
    uint d;
    uint max_lights;
} bitonic_vars;

void main()
{
    uvec3 total_invocation_grid = gl_NumWorkGroups * gl_WorkGroupSize;
    uint total_invocation_grid_size = uint(dot(total_invocation_grid, uvec3(1)));

    uint n =
        (gl_GlobalInvocationID.z * total_invocation_grid.x * total_invocation_grid.y +
        gl_GlobalInvocationID.y * total_invocation_grid.x +
        gl_GlobalInvocationID.x);

    if (n > bitonic_vars.level_size - 1)
    {
        return;
    }

    n = (bitonic_vars.level_size - 1) + n;

    //uint n = gl_GlobalInvocationID.x;
    uint node_d = bitonic_vars.level;
    //uint max_d = uint(floor(log2(input_ssbo.pbt.length())));
    uint target_d = bitonic_vars.d;

    uint left_leaf_node = uint((1 << target_d)) * n + (uint((1 << target_d)) - 1);
    uint right_leaf_node = uint((1 << target_d)) * n + (uint((1 << (target_d + 1))) - 2);

    uint leaf_node_dist = right_leaf_node - left_leaf_node;

    input_ssbo.pbt[n] = input_ssbo.pbt[left_leaf_node];

    for (uint leaf_node_index = left_leaf_node + 1; leaf_node_index <= right_leaf_node; ++leaf_node_index)
    {
        float bb_mix = step(0.001, input_ssbo.pbt[leaf_node_index].total_intensity.x);

        input_ssbo.pbt[n].bb.min_bounds = mix(input_ssbo.pbt[n].bb.min_bounds, min(input_ssbo.pbt[n].bb.min_bounds, input_ssbo.pbt[leaf_node_index].bb.min_bounds), bb_mix);
        input_ssbo.pbt[n].bb.max_bounds = mix(input_ssbo.pbt[n].bb.max_bounds, max(input_ssbo.pbt[n].bb.max_bounds, input_ssbo.pbt[leaf_node_index].bb.max_bounds), bb_mix);
        input_ssbo.pbt[n].total_intensity.x += input_ssbo.pbt[leaf_node_index].total_intensity.x;
    }

    input_ssbo.pbt[n].original_index = ivec4(n, left_leaf_node, right_leaf_node, 0);
    
}