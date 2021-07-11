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

    gl_Position = projection * view * (offset + (model * vec4(position * 6.0, 1.0)));
}