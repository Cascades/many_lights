#version 460 core
out vec4 FragColor;
 
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D texture_ambient1;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_bump1;
uniform sampler2D texture_dissolve1;

struct Light
{
    vec4 position;
    vec4 color;
};

layout (std140) uniform Lights {
    Light lights[100];
};

uniform int light_index;

void main()
{
    FragColor = lights[light_index].color;
} 