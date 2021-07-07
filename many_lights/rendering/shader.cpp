#include <glad/glad.h>

#include <string>
#include <filesystem>
#include <fstream>

#include "many_lights/shader.h"

#include <iostream>

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

ml::Shader::Shader(std::filesystem::path const& comp_path)
{
    std::string comp_code;

    try
    {
        file_to_string(comp_path, comp_code);
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "Could not read vertex shader:" << std::endl;
        std::cout << "|_" << e.what() << std::endl;
        std::cout << "|_" << e.code() << std::endl;
        throw e;
    }

    const char* comp_c_string_code = comp_code.c_str();

    // vertex shader
    GLuint const comp = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(comp, 1, &comp_c_string_code, nullptr);
    glCompileShader(comp);
    check_shader_compilation_errors(comp, GL_COMPUTE_SHADER);

    // shader Program
    id = std::make_shared<GLuint>(glCreateProgram());
    glAttachShader(*id, comp);
    glLinkProgram(*id);
    check_shader_program_linking_errors();

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(comp);

    index_uniforms();
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
        std::cout << "Could not read vertex shader:" << std::endl;
        std::cout << "|_" << e.what() << std::endl;
        std::cout << "|_" << e.code() << std::endl;
        throw e;
    }

    try
    {
        file_to_string(frag_path, frag_code);
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "Could not read fragment shader:" << std::endl;
        std::cout << "|_" << e.what() << std::endl;
        std::cout << "|_" << e.code() << std::endl;
        throw e;
    }

    const char* vert_c_string_code = vert_code.c_str();
    const char* frag_c_string_code = frag_code.c_str();

    // vertex shader
    GLuint const vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vert_c_string_code, nullptr);
    glCompileShader(vertex);
    check_shader_compilation_errors(vertex, GL_VERTEX_SHADER);

    // fragment Shader
    GLuint const fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &frag_c_string_code, nullptr);
    glCompileShader(fragment);
    check_shader_compilation_errors(fragment, GL_FRAGMENT_SHADER);

    // shader Program
    id = std::make_shared<GLuint>(glCreateProgram());
    glAttachShader(*id, vertex);
    glAttachShader(*id, fragment);
    glLinkProgram(*id);
    check_shader_program_linking_errors();

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    index_uniforms();
}

void ml::Shader::use() const
{
    glUseProgram(*id);
}

void ml::Shader::index_uniforms()
{
    GLint active_uniform_count;
    glGetProgramInterfaceiv(*id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &active_uniform_count);

    GLint length;
    GLenum prop = GL_NAME_LENGTH;
    std::string name;
	
    for (GLint active_uniform = 0; active_uniform < active_uniform_count; active_uniform++)
    {
        glGetProgramResourceiv(*id, GL_UNIFORM, active_uniform, 1, &prop, 1, nullptr, &length);

        name.resize(length);

        glGetProgramResourceName(*id, GL_UNIFORM, active_uniform, length, nullptr, name.data());

        name.pop_back();

        if (GLint location = glGetUniformLocation(*id, name.c_str()); location >= 0)
        {
            uniform_map.insert({ name, location });
        }
    }

	for(auto & x : uniform_map)
	{
        std::cout << x.first << " -> " << x.second << std::endl;
	}
}

void ml::Shader::check_shader_compilation_errors(GLuint const & shader, GLuint const & type)
{
    GLint success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        ml::ShaderCompilationException e(type);
        glGetShaderInfoLog(shader, 1024, nullptr, e.infoLog);
        std::cout << e.infoLog << std::endl;
        throw e;
    }
}

void ml::Shader::check_shader_program_linking_errors()
{
    GLint success;

    glGetShaderiv(*id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        ml::ShaderProgramLinkingException e{};
        glGetShaderInfoLog(*id, 1024, nullptr, e.infoLog);
        throw e;
    }
}

void ml::Shader::set_float(std::string const& location, GLfloat const& v0)
{
    glUniform1f(uniform_map.at(location), v0);
}

void ml::Shader::set_float2(std::string const& location, GLfloat const& v0, GLfloat const& v1)
{
    glUniform2f(uniform_map.at(location), v0, v1);
}

void ml::Shader::set_float3(std::string const& location, GLfloat const& v0, GLfloat const& v1, GLfloat const& v2)
{
    glUniform3f(uniform_map.at(location), v0, v1, v2);
}

void ml::Shader::set_float4(std::string const& location, GLfloat const& v0, GLfloat const& v1, GLfloat const& v2,
	GLfloat const& v3)
{
    glUniform4f(uniform_map.at(location), v0, v1, v2, v3);
}

void ml::Shader::set_int(std::string const& location, GLint const& v0)
{
    glUniform1i(uniform_map.at(location), v0);
}

void ml::Shader::set_int2(std::string const& location, GLint const& v0, GLint const& v1)
{
    glUniform2i(uniform_map.at(location), v0, v1);
}

void ml::Shader::set_int3(std::string const& location, GLint const& v0, GLint const& v1, GLint const& v2)
{
    glUniform3i(uniform_map.at(location), v0, v1, v2);
}

void ml::Shader::set_int4(std::string const& location, GLint const& v0, GLint const& v1, GLint const& v2,
	GLint const& v3)
{
    glUniform4i(uniform_map.at(location), v0, v1, v2, v3);
}

void ml::Shader::set_uint(std::string const& location, GLuint const& v0)
{
    glUniform1ui(uniform_map.at(location), v0);
}

void ml::Shader::set_uint2(std::string const& location, GLuint const& v0, GLuint const& v1)
{
    glUniform2ui(uniform_map.at(location), v0, v1);
}

void ml::Shader::set_uint3(std::string const& location, GLuint const& v0, GLuint const& v1, GLuint const& v2)
{
    glUniform3ui(uniform_map.at(location), v0, v1, v2);
}

void ml::Shader::set_uint4(std::string const& location, GLuint const& v0, GLuint const& v1, GLuint const& v2,
	GLuint const& v3)
{
    glUniform4ui(uniform_map.at(location), v0, v1, v2, v3);
}

void ml::Shader::set_floatv(std::string const& location, GLsizei const& count, GLfloat const* const& value)
{
    glUniform1fv(uniform_map.at(location), count, value);
}

void ml::Shader::set_floatv2(std::string const& location, GLsizei const& count, GLfloat const* const& value)
{
    glUniform2fv(uniform_map.at(location), count, value);
}

void ml::Shader::set_floatv3(std::string const& location, GLsizei const& count, GLfloat const* const& value)
{
    glUniform3fv(uniform_map.at(location), count, value);
}

void ml::Shader::set_floatv4(std::string const& location, GLsizei const& count, GLfloat const* const& value)
{
    glUniform4fv(uniform_map.at(location), count, value);
}

void ml::Shader::set_intv(std::string const& location, GLsizei const& count, GLint const* const& value)
{
    glUniform1iv(uniform_map.at(location), count, value);
}

void ml::Shader::set_intv2(std::string const& location, GLsizei const& count, GLint const* const& value)
{
    glUniform2iv(uniform_map.at(location), count, value);
}

void ml::Shader::set_intv3(std::string const& location, GLsizei const& count, GLint const* const& value)
{
    glUniform3iv(uniform_map.at(location), count, value);
}

void ml::Shader::set_intv4(std::string const& location, GLsizei const& count, GLint const* const& value)
{
    glUniform4iv(uniform_map.at(location), count, value);
}

void ml::Shader::set_uintv(std::string const& location, GLsizei const& count, GLuint const* const& value)
{
    glUniform1uiv(uniform_map.at(location), count, value);
}

void ml::Shader::set_uintv2(std::string const& location, GLsizei const& count, GLuint const* const& value)
{
    glUniform2uiv(uniform_map.at(location), count, value);
}

void ml::Shader::set_uintv3(std::string const& location, GLsizei const& count, GLuint const* const& value)
{
    glUniform3uiv(uniform_map.at(location), count, value);
}

void ml::Shader::set_uintv4(std::string const& location, GLsizei const& count, GLuint const* const& value)
{
    glUniform4uiv(uniform_map.at(location), count, value);
}

void ml::Shader::set_mat_2x2_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix2fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_3x3_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix3fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_4x4_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix4fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_2x3_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix2x3fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_3x2_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix3x2fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_2x4_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix2x4fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_4x2_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix4x2fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_3x4_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix3x4fv(uniform_map.at(location), count, transpose, value);
}

void ml::Shader::set_mat_4x3_floatv(std::string const& location, GLsizei const& count, GLboolean const& transpose,
	GLfloat const* const& value)
{
    glUniformMatrix4x3fv(uniform_map.at(location), count, transpose, value);
}
