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

layout(binding = 6) buffer MortonSSBO {
    uint morton_index_code[];
} morton_ssbo;

layout(binding = 7) buffer BitonicSSBO {
    uint stage;
    uint pass_num;
    uint num_lights;
    uint max_lights;
} bitonic_vars;

void main()
{
    // 0
    uint n = gl_GlobalInvocationID.x;

    uint max_stage = uint(log2(float(morton_ssbo.morton_index_code.length()))) - 1;

    uint array_size = morton_ssbo.morton_index_code.length();

    uint groups = 0;
    uint group_sizes = array_size;

    if (bitonic_vars.stage != max_stage)
    {
        groups = uint(pow(2.0, (max_stage - bitonic_vars.stage - 1)));
        group_sizes = array_size / groups;
    }

    uint stage_minus_pass = bitonic_vars.stage - bitonic_vars.pass_num;
    uint two_to_the_smp = uint(pow(2.0, float(stage_minus_pass)));
    uint two_to_the_smp_plus_one = uint(pow(2.0, float(stage_minus_pass + 1)));

    uint this_index = 2 * n - (n % two_to_the_smp);

    uint partner_index = this_index ^ two_to_the_smp;

    uint ordering;

    if (bitonic_vars.stage != max_stage)
    {
        uint curr_index = 2 * n - (n % two_to_the_smp);

        uint curr_group = curr_index / group_sizes;

        uint curr_half = (curr_index % group_sizes) / (group_sizes / 2);

        uint curr_cell_half = (curr_index % (1 << (bitonic_vars.stage - bitonic_vars.pass_num + 1))) / (1 << (bitonic_vars.stage - bitonic_vars.pass_num));

        if ((curr_half == 0 && curr_cell_half == 1) || (curr_half == 1 && curr_cell_half == 0))
        {
            ordering = 0;
        }
        else
        {
            ordering = 1;
        }
    }
    else
    {
        ordering = 0;
    }

    uint temp = this_index;

    this_index = this_index * (1 - ordering) + partner_index * ordering;

    partner_index = partner_index * (1 - ordering) + temp * ordering;

    // 
    if (morton_ssbo.morton_index_code[this_index] < morton_ssbo.morton_index_code[partner_index])
    {
        uint tmp = morton_ssbo.morton_index_code[this_index];
        morton_ssbo.morton_index_code[this_index] = morton_ssbo.morton_index_code[partner_index];
        morton_ssbo.morton_index_code[partner_index] = tmp;

        PBTNode pbt_tmp = input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + this_index];
        input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + this_index] = input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + partner_index];
        input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + partner_index] = pbt_tmp;
    }
}


/*void main()
{
    // 0
    uint n = gl_GlobalInvocationID.x;

    // 0
    uint stage_minus_pass = bitonic_vars.stage - bitonic_vars.pass_num;
    // 1
    uint two_to_the_smp = uint(pow(2.0f, float(stage_minus_pass)));
    // 2
    uint two_to_the_smp_plus_one = uint(pow(2.0f, float(stage_minus_pass + 1)));

    // 0
    uint this_index = 2 * n - (n % two_to_the_smp);

    // 1
    uint partner_index = this_index ^ two_to_the_smp;

    uint ordering;
    // nope
    if (bitonic_vars.stage != uint(log2((input_ssbo.pbt.length() + 1) / 2)))
    {
        ordering = ((this_index + two_to_the_smp) / two_to_the_smp_plus_one) % 2;
    }
    else
    {
        // 1
        ordering = ((this_index + two_to_the_smp) / two_to_the_smp) % 2;
    }

    // 0
    uint temp = this_index;

    // 1
    this_index = this_index * (1 - ordering) + partner_index * ordering;

    // 0
    partner_index = partner_index * (1 - ordering) + temp * ordering;

    // 
    if (morton_ssbo.morton_index_code[this_index] < morton_ssbo.morton_index_code[partner_index])
    {
        uint tmp = morton_ssbo.morton_index_code[this_index];
        morton_ssbo.morton_index_code[this_index] = morton_ssbo.morton_index_code[partner_index];
        morton_ssbo.morton_index_code[partner_index] = tmp;

        PBTNode pbt_tmp = input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + this_index];
        input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + this_index] = input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + partner_index];
        input_ssbo.pbt[(input_ssbo.pbt.length() / 2) + partner_index] = pbt_tmp;
    }
}*/