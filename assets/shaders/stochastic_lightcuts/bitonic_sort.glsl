#version 460

layout(local_size_x = 1, local_size_y = 1) in;

layout(std430) buffer;

layout(binding = 0) buffer InputSSBO {
    uint stage;
    uint pass_num;
	uint original_indicies[];
} input_ssbo;

void main()
{
    uint stage_minus_pass = stage - pass_num;
    uint two_to_the_smp = uint(pow(2.0f, float(stage_minus_pass)));
    uint two_to_the_smp_plus_one = uint(pow(2.0f, float(stage_minus_pass + 1)));

    uint this_index = 2 * n - (n % two_to_the_smp);

    uint partner_index = this_index ^ two_to_the_smp;

    uint ordering;
    if (stage != log2(array_size) - 1)
    {
        ordering = ((this_index + two_to_the_smp) / two_to_the_smp_plus_one) % 2;
    }
    else
    {
        ordering = ((this_index + two_to_the_smp) / two_to_the_smp) % 2;
    }

    uint temp = this_index;

    this_index = this_index * (1 - ordering) + partner_index * ordering;

    partner_index = partner_index * (1 - ordering) + temp * ordering;

    if (morton_list[this_index] < morton_list[partner_index])
    {
        uint tmp = morton_list[this_index];
        morton_list[this_index] = morton_list[partner_index];
        morton_list[partner_index] = tmp;
    }
}