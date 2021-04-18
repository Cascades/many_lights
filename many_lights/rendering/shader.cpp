#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <filesystem>
#include <fstream>

#include "many_lights/shader.h"
#include "many_lights/exceptions.h"

void ml::Shader::file_to_string(std::filesystem::path const& shader_path, std::string& out_string)
{
    std::ifstream shader_file_stream;

    shader_file_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    shader_file_stream.open(shader_path);

    std::stringstream shader_string_stream;

    shader_string_stream << shader_file_stream.rdbuf();

    shader_file_stream.close();

    out_string = shader_string_stream.str();
}

ml::Shader::Shader(std::filesystem::path const& vert_path, std::filesystem::path const& frag_path)
{
    std::string vert_code, frag_code;

    try
    {
        file_to_string(vert_path, vert_code);
    }
    catch (std::ifstream::failure& e)
    {
        throw "Could not compile vertex shader";
    }

    try
    {
        file_to_string(frag_path, frag_code);
    }
    catch (std::ifstream::failure& e)
    {
        throw "Could not compile fragment shader";
    }

    const char* vert_c_string_code = vert_code.c_str();
    const char* frag_c_string_code = frag_code.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;

    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vert_c_string_code, NULL);
    glCompileShader(vertex);
    check_shader_compilation_errors(vertex, GL_VERTEX_SHADER);

    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &frag_c_string_code, NULL);
    glCompileShader(fragment);
    check_shader_compilation_errors(fragment, GL_FRAGMENT_SHADER);

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    check_shader_program_linking_errors(ID);

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void ml::Shader::use()
{
    glUseProgram(ID);
}

void ml::Shader::check_shader_compilation_errors(GLuint shader, GLuint type)
{
    GLint success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        ml::ShaderCompilationException e(type);
        glGetShaderInfoLog(shader, 1024, NULL, e.infoLog);
        throw e;
    }
}

void ml::Shader::check_shader_program_linking_errors(GLuint shader)
{
    GLint success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        ml::ShaderProgramLinkingException e{};
        glGetShaderInfoLog(shader, 1024, NULL, e.infoLog);
        throw e;
    }
}