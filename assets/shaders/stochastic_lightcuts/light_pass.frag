#version 460 core

#define MAX_LIGHTS 200
#define MAX_LIGHTCUT_SIZE 20

out vec4 FragColor;

in vec2 TexCoords;

struct Light
{
    vec4 position;
    vec4 color;
};

uniform uint num_lights;

layout(std430) buffer;

layout (binding = 4) buffer Lights {
    Light lights[];
} lights;

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
    // CHANGE 
	PBTNode pbt[];
} input_ssbo;

layout(binding = 1) buffer LightcutsSSBO {
	int lightcuts[];
} lightcuts_ssbo;

layout(binding = 2) buffer MiscSSBO {
	vec3 viewPos;
    int iFrame;
    int lightcuts_size;
} misc_vars;

layout(binding = 3) buffer GridCellDebug {
	int lightcuts[];
} grid_cell_debug;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_diff_spec;
uniform sampler2D g_ambient;

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
    float dead_node_mult = step(0.0000001, input_ssbo.pbt[get_left_child_index(index)].total_intensity.x + input_ssbo.pbt[get_right_child_index(index)].total_intensity.x);
    return (index >= (input_ssbo.pbt.length() / 2)) && (dead_node_mult == 0.0);
}

float get_distance_squared(in vec3 shading_point, in vec3 other_point)
{
    vec3 dist_vec = other_point - shading_point;
    
    return dot(dist_vec, dist_vec);
}

float get_min_distance_squared(in vec3 shading_point, in int pbt_index)
{
    vec3 closest_point = clamp(shading_point, input_ssbo.pbt[pbt_index].bb.min_bounds.xyz, input_ssbo.pbt[pbt_index].bb.max_bounds.xyz); 

    return get_distance_squared(shading_point, closest_point);
}

float get_max_distance_squared(in vec3 shading_point, in int pbt_index)
{

    float max_dist = 0;

	float cands[8];

	cands[0] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.min_bounds.x, input_ssbo.pbt[pbt_index].bb.min_bounds.y, input_ssbo.pbt[pbt_index].bb.min_bounds.z)));
	cands[1] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.min_bounds.x, input_ssbo.pbt[pbt_index].bb.min_bounds.y, input_ssbo.pbt[pbt_index].bb.max_bounds.z)));
	cands[2] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.min_bounds.x, input_ssbo.pbt[pbt_index].bb.max_bounds.y, input_ssbo.pbt[pbt_index].bb.min_bounds.z)));
	cands[3] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.min_bounds.x, input_ssbo.pbt[pbt_index].bb.max_bounds.y, input_ssbo.pbt[pbt_index].bb.max_bounds.z)));
	cands[4] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.max_bounds.x, input_ssbo.pbt[pbt_index].bb.min_bounds.y, input_ssbo.pbt[pbt_index].bb.min_bounds.z)));
	cands[5] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.max_bounds.x, input_ssbo.pbt[pbt_index].bb.min_bounds.y, input_ssbo.pbt[pbt_index].bb.max_bounds.z)));
	cands[6] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.max_bounds.x, input_ssbo.pbt[pbt_index].bb.max_bounds.y, input_ssbo.pbt[pbt_index].bb.min_bounds.z)));
	cands[7] = get_distance_squared(shading_point, normalize(vec3(input_ssbo.pbt[pbt_index].bb.max_bounds.x, input_ssbo.pbt[pbt_index].bb.max_bounds.y, input_ssbo.pbt[pbt_index].bb.max_bounds.z)));

	for (int local_index = 0; local_index < 8; ++local_index)
	{
		max_dist = max(max_dist, cands[local_index]);
	}

    return max_dist;
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
	vec3 viewDir = normalize(misc_vars.viewPos - shading_pos);

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

float reflectance_bound(in int pbt_light_index, in vec3 shading_pos, in vec3 shading_ambient, in vec3 shading_diffuse, in float shading_specular, in vec3 shading_normal)
{
    return geo_term(pbt_light_index, shading_pos, shading_normal) * mat_term(pbt_light_index, shading_pos, shading_ambient, shading_diffuse, shading_specular, shading_normal);
}

float intensity(in int pbt_index)
{
    return input_ssbo.pbt[pbt_index].total_intensity.x;
}

int xorshift(in int value) {
    // Xorshift*32
    // Based on George Marsaglia's work: http://www.jstatsoft.org/v08/i14/paper
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    return value;
}

float hash1( uint n ) 
{
    // integer hash copied from Hugo Elias
	n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 789221U) + 1376312589U;
    return float( n & uvec3(0x7fffffffU))/float(0x7fffffff);
}

int sample_node(in int node_index, inout int seed, in vec3 shading_pos, in vec3 shading_ambient, in vec3 shading_diffuse, in float shading_specular, in vec3 shading_normal)
{
	int current_index = node_index;

    while(!node_is_leaf(current_index))
	{
        int left_child_index = get_left_child_index(current_index);
        int right_child_index = get_right_child_index(current_index);

        const float dead_node_value = 0.0;

        //float dead_node_mult = step(0.5, intensity(left_child_index) + intensity(right_child_index));

        /*if(intensity(left_child_index) == dead_node_value)
        {
            /*if(intensity(right_child_index) == dead_node_value)
            {
                break;
            }
            current_index = right_child_index;
            continue;
        }
        else if(intensity(right_child_index) == dead_node_value)
        {
            /*if(intensity(left_child_index) == dead_node_value)
            {
                break;
            }
            current_index = left_child_index;
            continue;
        }*/

        float ref_bound_and_intense_l = reflectance_bound(left_child_index, shading_pos, shading_ambient, shading_diffuse, shading_specular, shading_normal) * intensity(left_child_index);
        float ref_bound_and_intense_r = reflectance_bound(right_child_index, shading_pos, shading_ambient, shading_diffuse, shading_specular, shading_normal) * intensity(right_child_index);

        if (ref_bound_and_intense_l + ref_bound_and_intense_r == dead_node_value)
        {
            break;
        }

        if(ref_bound_and_intense_l == dead_node_value)
        {
            current_index = right_child_index;
            continue;
        }
        else if(ref_bound_and_intense_r == dead_node_value)
        {
            current_index = left_child_index;
            continue;
        }

		float w_min_l = ref_bound_and_intense_l / get_min_distance_squared(shading_pos, left_child_index);
        float w_max_l = ref_bound_and_intense_l / get_max_distance_squared(shading_pos, left_child_index);

        float w_min_r = ref_bound_and_intense_r / get_min_distance_squared(shading_pos, right_child_index);
        float w_max_r = ref_bound_and_intense_r / get_max_distance_squared(shading_pos, right_child_index);

        float p_min_l = w_min_l / (w_min_l + w_min_r);
        float p_max_l = w_min_l / (w_min_l + w_min_r);

        float p_l = (p_min_l + p_max_l) / 2.0;

        float rngSeed = hash1(seed);
    
        seed = abs(xorshift(int(rngSeed * 100000000.0)));

        float ran_val = clamp(fract(float(seed) / 3141.592653), 0.0, 1.0);

        if(ran_val < p_l)
        {
            current_index = left_child_index;
        }
        else
        {
            current_index = right_child_index;
        }
	}

    return current_index;
}

void main()
{
    vec3 result = vec3(0.0, 0.0, 0.0);

    float ambientStrength = 0.1;
    
    vec3 FragPos = texture(g_position, TexCoords).rgb;
    vec3 norm = texture(g_normal, TexCoords).rgb;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 lightDir;
    float diff;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(misc_vars.viewPos - FragPos);
    vec3 reflectDir;  
    float spec;

    float dist;
    float attenuation;

    float a = 0.001;
    float b = 0.0001;

    vec2 screen_coord = gl_FragCoord.xy;

    uint seed = uint(screen_coord.x) + uint(800U*screen_coord.y) + (800U*600U)*uint(misc_vars.iFrame);

    ivec2 grid_coord = ivec2(screen_coord / 16.0);

    //int final_light_indices[MAX_LIGHTCUT_SIZE];

    for(int cut_index = 0; cut_index < misc_vars.lightcuts_size; ++cut_index)
    {
        int light_index = lightcuts_ssbo.lightcuts[((grid_coord.y * 51 + grid_coord.x) * MAX_LIGHTCUT_SIZE) + cut_index];

        int final_light_index = sample_node(light_index, seed, texture(g_position, TexCoords).rgb, texture(g_ambient, TexCoords).rgb, texture(g_diff_spec, TexCoords).rgb, texture(g_diff_spec, TexCoords).a, texture(g_normal, TexCoords).rgb);
        final_light_index = input_ssbo.pbt[final_light_index].original_index.r;

        //final_light_indices[cut_index] = final_light_index;

        dist = distance(FragPos, lights.lights[final_light_index].position.xyz);
        attenuation = 1.0 / (1.0 + a*dist + b*dist*dist);

        // ambient0
        ambient = ambientStrength * lights.lights[final_light_index].color.rgb * texture(g_ambient, TexCoords).rgb;
  	
        // diffuse0
        lightDir = normalize(lights.lights[final_light_index].position.xyz - FragPos);
        diff = max(dot(norm, lightDir), 0.0);
        diffuse = diff * lights.lights[final_light_index].color.rgb * texture(g_diff_spec, TexCoords).rgb;
    
        // specular0
        reflectDir = reflect(-lightDir, norm);  
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        specular = specularStrength * spec * lights.lights[final_light_index].color.rgb * vec3(texture(g_diff_spec, TexCoords).a);

        vec3 light_sum = ambient + diffuse + specular;

        //float scaling_factor = input_ssbo.pbt[light_index].total_intensity.x / (dot(vec3(1.0), light_sum) / 3.0);

        float scaling_factor = 1.0;

        result += attenuation * (ambient + diffuse + specular) * scaling_factor;
    }

    //if(grid_coord == ivec2(25, 15))
    //{
    //    int grid_x = int(screen_coord.x) % 16;
    //   int grid_y = int(screen_coord.y) % 16;
    //    for(int j = 0; j < MAX_LIGHTCUT_SIZE; ++j)
   //     {
    //        grid_cell_debug.lightcuts[(grid_y * 16 + grid_x) * (MAX_LIGHTCUT_SIZE + 3) + j] = final_light_indices[j];
     /*   }
        grid_cell_debug.lightcuts[(grid_y * 16 + grid_x) * (MAX_LIGHTCUT_SIZE + 3) + MAX_LIGHTCUT_SIZE] = int(result.r * 255);
        grid_cell_debug.lightcuts[(grid_y * 16 + grid_x) * (MAX_LIGHTCUT_SIZE + 3) + MAX_LIGHTCUT_SIZE + 1] = int(result.g * 255);
        grid_cell_debug.lightcuts[(grid_y * 16 + grid_x) * (MAX_LIGHTCUT_SIZE + 3) + MAX_LIGHTCUT_SIZE + 2] = int(result.b * 255);

        //grid_cell_debug.lightcuts[0] = input_ssbo.pbt[337].bb.min_bounds.x;

        result.r = 1.0;
    }*/

    FragColor = vec4(result, 1.0);
} 