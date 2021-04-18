#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#include "many_lights/exceptions.h"

const char* ml::ShaderCompilationException::what() const throw ()
{
    switch (shader_type)
    {
    case GL_FRAGMENT_SHADER:
    {
        char str[1024];
        strcpy(str, "Fragment shader compilation failed.\n");
        strcpy(str, infoLog);
        return str;
        break;
    }
    case GL_VERTEX_SHADER:
    {
        char str[1024];
        strcpy(str, "Vertex shader compilation failed.\n");
        strcpy(str, infoLog);
        return str;
        break;
    }
    default:
    {
        char str[1024];
        strcpy(str, "Unknown shader compilation failed.\n");
        strcpy(str, infoLog);
        return str;
        break;
    }
    }
}

const char* ml::ShaderProgramLinkingException::what() const throw ()
{
    return "Shader program linking failed.";
}