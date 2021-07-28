#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 offset;
layout (location = 2) in vec4 color;

out vec4 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fColor = color;

    if(model[3][3] == 1.0)
    {
        vec3 idk = position * 6.0 + offset.xyz;

        gl_Position = projection * view * vec4(idk, 1.0);
    }
    else
    {
        gl_Position = vec4(1.0);
    }
}