#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
  
uniform vec3 viewPos;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_diff_spec;
uniform sampler2D g_ambient;

layout(binding = 1, offset = 0) uniform atomic_uint lighting_comps;

struct Light
{
    vec4 position;
    vec4 color;
};

layout(std430) buffer;

layout (binding = 4) buffer Lights {
    Light lights[];
} lights;

uniform uint num_lights;
uniform int render_mode;

float near = 1.0;
float far = 10000.0;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{
    vec3 result = vec3(0.0, 0.0, 0.0);

    if(render_mode == 0)
    {
        result = texture(g_position, TexCoords).rgb;
    }
    else if(render_mode == 1)
    {
        result = texture(g_normal, TexCoords).rgb;
    }
    else if(render_mode == 2)
    {
        result = texture(g_diff_spec, TexCoords).rgb;
    }
    else if(render_mode == 3)
    {
        result = vec3(texture(g_diff_spec, TexCoords).a);
    }
    else if(render_mode == 4)
    {
        result = texture(g_ambient, TexCoords).rgb;
    }
    else if(render_mode == 5)
    {
        vec3 initial_col;
        initial_col.r = texture(g_position, TexCoords).a;
        initial_col.g = texture(g_normal, TexCoords).a;
        initial_col.b = texture(g_ambient, TexCoords).a;
        float ambientStrength = 0.1;
    
        vec3 FragPos = texture(g_position, TexCoords).rgb;
        vec3 norm = texture(g_normal, TexCoords).rgb;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;

        vec3 lightDir;
        float diff;

        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir;  
        float spec;

        float dist;
        float attenuation;

        float a = 0.001;
        float b = 0.0001;

        vec3 tex_ambient = texture(g_ambient, TexCoords).rgb;
        vec3 tex_diffuse = texture(g_diff_spec, TexCoords).rgb;
        vec3 tex_specular = vec3(texture(g_diff_spec, TexCoords).a);

        for(int light_index = 0; light_index < num_lights; ++light_index)
        {
            atomicCounterIncrement(lighting_comps);

            dist = distance(FragPos, lights.lights[light_index].position.xyz);
            attenuation = 1.0 / (1.0 + a*dist + b*dist*dist);

            // ambient0
            ambient = ambientStrength * lights.lights[light_index].color.rgb * tex_ambient;
  	
            // diffuse0
            lightDir = normalize(lights.lights[light_index].position.xyz - FragPos);
            diff = max(dot(norm, lightDir), 0.0);
            diffuse = diff * lights.lights[light_index].color.rgb * tex_diffuse;
    
            // specular0
            reflectDir = reflect(-lightDir, norm);  
            spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            specular = specularStrength * spec * lights.lights[light_index].color.rgb * tex_specular;

            result += attenuation * (ambient + diffuse + specular);
        }

        result *= initial_col;
    }

    FragColor = vec4(result, 1.0);
} 