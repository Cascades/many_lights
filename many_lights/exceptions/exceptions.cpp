#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#include "many_lights/exceptions.h"

const char* ml::ShaderCompilationException::what() const noexcept
{
    return infoLog;
}

const char* ml::ShaderProgramLinkingException::what() const noexcept
{
    return "Shader program linking failed.";
}