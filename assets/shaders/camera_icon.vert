#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;

struct Light
{
    vec4 position;
    vec4 color;
};

layout (std140) uniform Lights {
    Light lights[100];
};

layout (location = 1) uniform mat4 view;
layout (location = 2) uniform mat4 projection;
layout (location = 3) uniform int light_index;

void main()
{
    FragPos = vec3(lights[light_index].position.xyz + aPos);
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}