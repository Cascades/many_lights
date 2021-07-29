#version 460 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;
in vec2 TexCoords;

layout(location = 5) uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = vec4(texture(texture_diffuse1, TexCoords).rgb, 1.0);
} 