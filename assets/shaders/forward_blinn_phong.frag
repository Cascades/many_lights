#version 460 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;
in vec2 TexCoords;
  
uniform vec3 viewPos; 
uniform vec3 objectColor;

layout(location = 4) uniform sampler2D texture_ambient1;
layout(location = 5) uniform sampler2D texture_diffuse1;
layout(location = 6) uniform sampler2D texture_specular1;
layout(location = 7) uniform sampler2D texture_bump1;
layout(location = 8) uniform sampler2D texture_dissolve1;
layout(location = 9) uniform vec2 sampler_size;

layout(binding = 0, offset = 0) uniform atomic_uint lighting_comps;

struct Light
{
    vec4 position;
    vec4 color;
};

//layout (std140) uniform Lights {
//    Light lights[400];
//};

layout(std430) buffer;

layout (binding = 4) buffer Lights {
    Light lights[];
} lights;

uniform uint num_lights;

void main()
{
    vec3 result = vec3(0.0, 0.0, 0.0);

    if(texture(texture_dissolve1, TexCoords).r < 0.1)
    {
        discard;
    }

    float ambientStrength = 0.1;
    vec3 ambient;

    vec2 offset = vec2(1.0) / sampler_size * 25;

    float left = texture(texture_bump1, vec2((TexCoords.x - offset.x), TexCoords.y)).r;
    float right = texture(texture_bump1, vec2((TexCoords.x + offset.x), TexCoords.y)).r;
    float down = texture(texture_bump1, vec2(TexCoords.x, (TexCoords.y + offset.y))).r;
    float up = texture(texture_bump1, vec2(TexCoords.x, (TexCoords.y - offset.y))).r;

    vec3 bump_norm;
    bump_norm.x = left - right;
    bump_norm.y = 2.0;
    bump_norm.z = down - up;
    bump_norm = normalize(bump_norm);

    vec4 q;
    q.xyz = cross(bump_norm, vec3(0.0, 1.0, 0.0));
    q.w = dot(bump_norm, vec3(0.0, 1.0, 0.0));
    q = normalize(q);

    vec3 norm = normalize(Normal);

    // Extract the vector part of the quaternion
    vec3 u = vec3(q.x, q.y, q.z);

    // Extract the scalar part of the quaternion
    float s = q.w;

    // Do the math
    norm = 2.0 * dot(u, norm) * u + (s*s - dot(u, u)) * norm + 2.0 * s * cross(u, norm);

    vec3 lightDir;
    float diff;
    vec3 diffuse;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir;  
    float spec;
    vec3 specular;

    float dist;
    float attenuation;

    float a = 0.001;
    float b = 0.0001;

    for(int light_index = 0; light_index < num_lights; ++light_index)
    {
        atomicCounterIncrement(lighting_comps);

        dist = distance(FragPos, lights.lights[light_index].position.xyz);
        attenuation = 1.0 / (1.0 + a*dist + b*dist*dist);

        // ambient0
        ambient = ambientStrength * lights.lights[light_index].color.rgb * texture(texture_ambient1, TexCoords).rgb;
  	
        // diffuse0
        lightDir = normalize(lights.lights[light_index].position.xyz - FragPos);
        diff = max(dot(norm, lightDir), 0.0);
        diffuse = diff * lights.lights[light_index].color.rgb * texture(texture_diffuse1, TexCoords).rgb;
    
        // specular0
        reflectDir = reflect(-lightDir, norm);  
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        specular = specularStrength * spec * lights.lights[light_index].color.rgb * texture(texture_specular1, TexCoords).rgb;

        vec3 out_col = attenuation * (ambient + diffuse + specular);

        result += step(0.0001, out_col) * out_col;
    }

    FragColor = vec4(result, 1.0);
} 