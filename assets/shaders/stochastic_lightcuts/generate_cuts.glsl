#version 460

#define MAX_LIGHTCUT_SIZE 30

layout(local_size_x = 32, local_size_y = 32) in;
uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_diff_spec;
uniform sampler2D g_ambient;

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

layout(binding = 1) buffer OutputSSBO {
	int lightcuts[];
} output_ssbo;

layout(binding = 2) buffer MiscSSBO {
	vec4 viewPos;
	uvec2 screen_size;
	int iFrame;
	int lightcuts_size;
	int tile_size;
	float random_tile_sample;

} misc_vars;


struct BinaryHeap
{
	int count;
	float metric[MAX_LIGHTCUT_SIZE];
	int data[MAX_LIGHTCUT_SIZE];
};

int get_parent_index(int i)
{
	return int(floor((float(i) - 1.0) / 2.0));
}

int get_left_child_index(int i)
{
	return i * 2 + 1;
}

int get_right_child_index(int i)
{
	return i * 2 + 2;
}

bool node_is_leaf(int index)
{
	//float dead_node_mult = step(0.0000001, input_ssbo.pbt[get_left_child_index(index)].total_intensity.x + input_ssbo.pbt[get_right_child_index(index)].total_intensity.x);
	return index >= (input_ssbo.pbt.length() / 2);
}

void insert_node(inout BinaryHeap b, int d, float m)
{
	int i = b.count;
	b.metric[i] = m;
	b.data[i] = d;

	if (i != 0)
	{
		while (b.metric[i] > b.metric[get_parent_index(i)])
		{
			int parent_index = get_parent_index(i);

			b.metric[i] = b.metric[parent_index];
			b.metric[parent_index] = m;
			
			b.data[i] = b.data[parent_index];
			b.data[parent_index] = d;
			
			i = parent_index;
			if (i == 0)
			{
				break;
			}
		}
	}
	
	b.count += 1;
}

void delete_root(inout BinaryHeap b)
{
	b.metric[0] = b.metric[b.count - 1];
	b.metric[b.count - 1] = -1;

	b.data[0] = b.data[b.count - 1];
	b.data[b.count - 1] = -1;

	b.count--;

	if (b.count >= 2)
	{
		int i = 0;
		int largest = i;

		int left_index = get_left_child_index(i);
		int right_index = get_right_child_index(i);

		while (get_left_child_index(i) < b.count || get_right_child_index(i) < b.count)
		{

			if (get_left_child_index(i) < b.count)
			{
				if (b.metric[get_left_child_index(i)] > b.metric[largest])
				{
					largest = get_left_child_index(i);
				}
			}
			if (get_right_child_index(i) < b.count)
			{
				if (b.metric[get_right_child_index(i)] > b.metric[largest])
				{
					largest = get_right_child_index(i);
				}
			}

			if (largest == i)
			{
				break;
			}
			else
			{
				float m_tmp = b.metric[i];
				b.metric[i] = b.metric[largest];
				b.metric[largest] = m_tmp;

				int d_tmp = b.data[i];
				b.data[i] = b.data[largest];
				b.data[largest] = d_tmp;

				i = largest;
			}
		}
	}
}

//taken from DQLin
float max_dist_along(vec3 shading_pos, vec3 shading_norm, vec3 boundMin, vec3 boundMax)
{
	vec3 dir_p = shading_norm * shading_pos;
	vec3 mx0 = shading_norm * boundMin - dir_p;
	vec3 mx1 = shading_norm * boundMax - dir_p;
	return max(mx0[0], mx1[0]) + max(mx0[1], mx1[1]) + max(mx0[2], mx1[2]);
}

float geo_term(in int pbt_light_index, in vec3 shading_pos, in vec3 shading_normal)
{
	// not OG lightcuts
	//vec3 closest_light_pos = clamp(shading_pos, input_ssbo.pbt[pbt_light_index].bb.min_bounds.xyz, input_ssbo.pbt[pbt_light_index].bb.max_bounds.xyz);

	//vec3 dist_vec = closest_light_pos - shading_pos;

	//return dist_vec;

	//taken from DQLin
	float nrm_max = max_dist_along(shading_pos, shading_normal, input_ssbo.pbt[pbt_light_index].bb.min_bounds.xyz, input_ssbo.pbt[pbt_light_index].bb.max_bounds.xyz);
	vec3 d = min(max(shading_pos, input_ssbo.pbt[pbt_light_index].bb.min_bounds.xyz), input_ssbo.pbt[pbt_light_index].bb.max_bounds.xyz) - shading_pos;
	vec3 tng = d - dot(d, shading_normal) * shading_normal;
	float hyp2 = dot(tng, tng) + nrm_max * nrm_max;
	return step(0.0, nrm_max) * nrm_max * inversesqrt(hyp2);
}

float mat_term(in int pbt_light_index, in vec3 shading_pos, in vec3 shading_ambient, in vec3 shading_diffuse, in float shading_specular, in vec3 shading_normal)
{
	// can be taken out of here
	vec3 viewDir = normalize(vec3(misc_vars.viewPos) - shading_pos);

	float specularStrength = 0.5;
	float ambientStrength = 0.2;

	// ambient0
	vec3 ambient = ambientStrength * (input_ssbo.pbt[pbt_light_index].total_intensity.x / 3.0) * shading_ambient;

	// diffuse0

	float diff = 0;

	float cands[8];

	cands[0] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[1] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));
	cands[2] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[3] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));
	cands[4] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.max_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[5] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.max_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));
	cands[6] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.max_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[7] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.max_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));

	for (int local_index = 0; local_index < 8; ++local_index)
	{
		diff = max(diff, cands[local_index]);
	}
	
	vec3 diffuse = diff * (input_ssbo.pbt[pbt_light_index].total_intensity.x / 3.0) * shading_diffuse;

	// specular0
	//vec3 reflectDir = reflect(-lightDir, shading_normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	//vec3 specular = vec3(specularStrength * spec * (input_ssbo.pbt[pbt_light_index].total_intensity.x / 3.0) * shading_specular);
	vec3 specular = vec3(0.0);

	return max(1.0, dot(vec3(1.0), ambient + diffuse + specular) / 3.0);
}

float error_bound(in int pbt_light_index, in vec3 shading_pos, in vec3 shading_ambient, in vec3 shading_diffuse, in float shading_specular, in vec3 shading_normal)
{
	return geo_term(pbt_light_index, shading_pos, shading_normal) * mat_term(pbt_light_index, shading_pos, shading_ambient, shading_diffuse, shading_specular, shading_normal) * input_ssbo.pbt[pbt_light_index].total_intensity.x;
}

int xorshift(in int value) {
	// Xorshift*32
	// Based on George Marsaglia's work: http://www.jstatsoft.org/v08/i14/paper
	value ^= value << 13;
	value ^= value >> 17;
	value ^= value << 5;
	return value;
}

float hash1(uint n)
{
	// integer hash copied from Hugo Elias
	n = (n << 13U) ^ n;
	n = n * (n * n * 15731U + 789221U) + 1376312589U;
	return float(n & uvec3(0x7fffffffU)) / float(0x7fffffff);
}

void main()
{
	uvec2 pixel_coords = uvec2(gl_GlobalInvocationID.xy);

	if (any(greaterThanEqual(pixel_coords, misc_vars.screen_size)))
	{
		return;
	}

	vec2 top_left_of_tile = vec2(pixel_coords * misc_vars.tile_size);

	uint seed = uint(top_left_of_tile.x) + uint(misc_vars.screen_size.x * top_left_of_tile.y) + (misc_vars.screen_size.x * misc_vars.screen_size.y) * uint(misc_vars.iFrame);

	float rngSeed = hash1(seed);
	seed = abs(xorshift(int(rngSeed * 100000000.0)));
	float ran_val_x = clamp(fract(float(seed) / 3141.592653), 0.0, 1.0);
	rngSeed = hash1(seed);
	seed = abs(xorshift(int(rngSeed * 100000000.0)));
	float ran_val_y = clamp(fract(float(seed) / 3141.592653), 0.0, 1.0);

	vec2 ran_offset = vec2(ran_val_x, ran_val_y) * (1.0 - misc_vars.random_tile_sample);

	vec2 sampler_coords = (top_left_of_tile + (ran_offset * misc_vars.tile_size)) / vec2(misc_vars.screen_size.x, misc_vars.screen_size.y);

	uint output_index = (pixel_coords.y * ((misc_vars.screen_size.x / misc_vars.tile_size) + 1) + pixel_coords.x) * MAX_LIGHTCUT_SIZE;

	vec3 pos = texture(g_position, sampler_coords).xyz;
	vec3 norm = texture(g_normal, sampler_coords).xyz;
	vec3 amb = texture(g_ambient, sampler_coords).xyz;
	vec3 diff = texture(g_diff_spec, sampler_coords).xyz;
	float spec = texture(g_diff_spec, sampler_coords).w;

	//will break
	BinaryHeap lightcut_heap;

	lightcut_heap.count = 0;

	for (int a = 0; a < MAX_LIGHTCUT_SIZE; ++a)
	{
		lightcut_heap.data[a] = -1;
		lightcut_heap.metric[a] = -1;
	}

	float root_error_bound = error_bound(0, pos, amb, diff, spec, norm);

	insert_node(lightcut_heap, 0, root_error_bound);

	while (lightcut_heap.count < misc_vars.lightcuts_size)
	{
		int pbr_light_index = lightcut_heap.data[0];

		int left_child_index = get_left_child_index(pbr_light_index);
		int right_child_index = get_right_child_index(pbr_light_index);

		float left_error_bound;
		float right_error_bound;

		if (node_is_leaf(left_child_index))
		{
			left_error_bound = -1;
		}
		else
		{
			left_error_bound = error_bound(left_child_index, pos, amb, diff, spec, norm);
		}

		if (node_is_leaf(right_child_index))
		{
			right_error_bound = -1;
		}
		else
		{
			right_error_bound = error_bound(right_child_index, pos, amb, diff, spec, norm);
		}

		delete_root(lightcut_heap);

		insert_node(lightcut_heap, left_child_index, left_error_bound);
		insert_node(lightcut_heap, right_child_index, right_error_bound);
	}

	const bool data = true;

	if (data)
	{
		for (int i = 0; i < misc_vars.lightcuts_size; ++i)
		{
			output_ssbo.lightcuts[output_index + i] = lightcut_heap.data[i];
		}
	}
	else
	{
		for (int i = 0; i < misc_vars.lightcuts_size; ++i)
		{
			output_ssbo.lightcuts[output_index + i] = int(10 * lightcut_heap.metric[i]);
		}
	}

	//output_ssbo.lightcuts[output_index] = int(pixel_coords.y);
}