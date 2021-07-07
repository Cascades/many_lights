#version 460

#define MAX_LIGHTS 200
#define LIGHTCUT_SIZE 6

layout(local_size_x = 1, local_size_y = 1) in;
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
};

layout(binding = 0) buffer InputSSBO {
	PBTNode pbt[MAX_LIGHTS];
} input_ssbo;

layout(binding = 1) buffer OutputSSBO {
	int lightcuts[51 * 38 * LIGHTCUT_SIZE];
} output_ssbo;

layout(binding = 2) buffer MiscSSBO {
	vec3 viewPos;
	int iFrame;
} misc_vars;


struct BinaryHeap
{
	int count;
	float metric[LIGHTCUT_SIZE];
	int data[LIGHTCUT_SIZE];
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

float geo_term(in int pbt_light_index, in vec3 shading_pos)
{
	// not OG lightcuts
	vec3 closest_light_pos = clamp(shading_pos, input_ssbo.pbt[pbt_light_index].bb.min_bounds.xyz, input_ssbo.pbt[pbt_light_index].bb.max_bounds.xyz);

	vec3 dist_vec = closest_light_pos - shading_pos;

	return 1.0 / dot(dist_vec, dist_vec);
}

float mat_term(in int pbt_light_index, in vec3 shading_pos, in vec3 shading_ambient, in vec3 shading_diffuse, in float shading_specular, in vec3 shading_normal)
{
	// can be taken out of here
	vec3 viewDir = normalize(misc_vars.viewPos - shading_pos);

	float specularStrength = 0.5;
	float ambientStrength = 0.2;

	// ambient0
	vec3 ambient = ambientStrength * (input_ssbo.pbt[pbt_light_index].total_intensity.x / 3.0) * shading_ambient;

	// diffuse0
	//vec3 lightDir = normalize(input_ssbo.pbt[pbt_light_index].position.xyz - shading_pos);
	//float max_z = minput_ssbo.pbt[pbt_light_index].bb.max_bounds.z;
	//float diff = max_z;
	//if (max_z >= 0)
	//{
	//	float denom = pow(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, 2.0);
	//	denom += pow(input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, 2.0);
	//	denom += max_z;
	/*
		denom = sqrt(denom);

		diff /= sqrt(denom);
	}
	else
	{
		float denom = pow(input_ssbo.pbt[pbt_light_index].bb.max_bounds.x, 2.0);
		denom += pow(input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, 2.0);
		denom += max_z;

		denom = sqrt(denom);

		diff /= sqrt(denom);
	}*/

	float diff = 0;

	float cands[8];

	cands[0] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[1] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));
	cands[2] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[3] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));
	cands[4] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[5] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.min_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));
	cands[6] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.min_bounds.z) - shading_pos));
	cands[7] = dot(shading_normal, normalize(vec3(input_ssbo.pbt[pbt_light_index].bb.min_bounds.x, input_ssbo.pbt[pbt_light_index].bb.max_bounds.y, input_ssbo.pbt[pbt_light_index].bb.max_bounds.z) - shading_pos));

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

	return dot(vec3(1.0), ambient + diffuse + specular) / 3.0;
}

float error_bound(in int pbt_light_index, in vec3 shading_pos, in vec3 shading_ambient, in vec3 shading_diffuse, in float shading_specular, in vec3 shading_normal)
{
	return geo_term(pbt_light_index, shading_pos) * mat_term(pbt_light_index, shading_pos, shading_ambient, shading_diffuse, shading_specular, shading_normal) * input_ssbo.pbt[pbt_light_index].total_intensity.x;
}

void main()
{
	uvec2 pixel_coords = uvec2(gl_GlobalInvocationID.xy);

	vec2 sampler_coords = vec2(pixel_coords * 16) / vec2(800,600);

	uint output_index = (pixel_coords.y * 51 + pixel_coords.x) * LIGHTCUT_SIZE;

	vec3 pos = texture(g_position, sampler_coords).xyz;
	vec3 norm = texture(g_normal, sampler_coords).xyz;
	vec3 amb = texture(g_ambient, sampler_coords).xyz;
	vec3 diff = texture(g_diff_spec, sampler_coords).xyz;
	float spec = texture(g_diff_spec, sampler_coords).w;

	//will break
	BinaryHeap lightcut_heap = BinaryHeap(0, float[LIGHTCUT_SIZE](-1.0, -1.0, -1.0, -1.0, -1.0, -1.0), int[LIGHTCUT_SIZE](-1, -1, -1, -1, -1, -1));

	float root_error_bound = error_bound(0, pos, amb, diff, spec, norm);

	insert_node(lightcut_heap, 0, root_error_bound);

	while (lightcut_heap.count < 6)
	{
		int pbr_light_index = lightcut_heap.data[0];

		int left_child_index = get_left_child_index(pbr_light_index);
		int right_child_index = get_right_child_index(pbr_light_index);

		float left_error_bound = error_bound(left_child_index, pos, amb, diff, spec, norm);
		float right_error_bound = error_bound(right_child_index, pos, amb, diff, spec, norm);

		delete_root(lightcut_heap);

		insert_node(lightcut_heap, left_child_index, left_error_bound);
		insert_node(lightcut_heap, right_child_index, right_error_bound);
	}

	const bool data = true;

	if (data)
	{
		output_ssbo.lightcuts[output_index + 0] = lightcut_heap.data[0];
		output_ssbo.lightcuts[output_index + 1] = lightcut_heap.data[1];
		output_ssbo.lightcuts[output_index + 2] = lightcut_heap.data[2];
		output_ssbo.lightcuts[output_index + 3] = lightcut_heap.data[3];
		output_ssbo.lightcuts[output_index + 4] = lightcut_heap.data[4];
		output_ssbo.lightcuts[output_index + 5] = lightcut_heap.data[5];
	}
	else
	{
		output_ssbo.lightcuts[output_index + 0] = int(10 * lightcut_heap.metric[0]);
		output_ssbo.lightcuts[output_index + 1] = int(10 * lightcut_heap.metric[1]);
		output_ssbo.lightcuts[output_index + 2] = int(10 * lightcut_heap.metric[2]);
		output_ssbo.lightcuts[output_index + 3] = int(10 * lightcut_heap.metric[3]);
		output_ssbo.lightcuts[output_index + 4] = int(10 * lightcut_heap.metric[4]);
		output_ssbo.lightcuts[output_index + 5] = int(10 * lightcut_heap.metric[5]);
	}

	//output_ssbo.lightcuts[output_index] = int(pixel_coords.y);
}