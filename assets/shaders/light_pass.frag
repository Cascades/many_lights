#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
  
uniform vec3 viewPos;

uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_diff_spec;
uniform sampler2D g_ambient;

struct Light
{
    vec4 position;
    vec4 color;
};

layout (std140) uniform Lights {
    Light lights[100];
};

uniform uint num_lights;

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
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir;  
    float spec;

    float dist;
    float attenuation;

    float a = 0.001;
    float b = 0.0001;

    for(int light_index = 0; light_index < num_lights; ++light_index)
    {
        dist = distance(FragPos, lights[light_index].position.xyz);
        attenuation = 1.0 / (1.0 + a*dist + b*dist*dist);

        // ambient0
        ambient = ambientStrength * lights[light_index].color.rgb * texture(g_ambient, TexCoords).rgb;
  	
        // diffuse0
        lightDir = normalize(lights[light_index].position.xyz - FragPos);
        diff = max(dot(norm, lightDir), 0.0);
        diffuse = diff * lights[light_index].color.rgb * texture(g_diff_spec, TexCoords).rgb;
    
        // specular0
        reflectDir = reflect(-lightDir, norm);  
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        specular = specularStrength * spec * lights[light_index].color.rgb * vec3(texture(g_diff_spec, TexCoords).a);

        result += attenuation * (ambient + diffuse + specular);
    }

    FragColor = vec4(result, 1.0);
} 