#pragma once
#include <glad/glad.h>

#include <string>
#include <filesystem>
#include <limits>

#include <map>

namespace ml
{
    class Shader
    {
    private:
        std::map<std::string, GLint> uniform_map;
    	
    public:
        Shader()
        {
            id = std::make_shared<GLuint>(std::numeric_limits<GLuint>::max());
        }

        std::shared_ptr<GLuint> id;

        void file_to_string(std::filesystem::path const& shader_path, std::string& out_string);

        Shader(std::filesystem::path const& vert_path, std::filesystem::path const& frag_path);

        void use() const;

    private:
        void index_uniforms();
    	
        void check_shader_compilation_errors(GLuint const& shader, GLuint const& type);

        void check_shader_program_linking_errors();

    public:
        void set_float(std::string const & location, GLfloat const & v0);
        void set_float2(std::string const & location, GLfloat const & v0, GLfloat const & v1);
        void set_float3(std::string const & location, GLfloat const & v0, GLfloat const & v1, GLfloat const & v2);
        void set_float4(std::string const & location, GLfloat const & v0, GLfloat const & v1, GLfloat const & v2, GLfloat const & v3);
        void set_int(std::string const & location, GLint const & v0);
        void set_int2(std::string const & location, GLint const & v0, GLint const & v1);
        void set_int3(std::string const & location, GLint const & v0, GLint const & v1, GLint const & v2);
        void set_int4(std::string const & location, GLint const & v0, GLint const & v1, GLint const & v2, GLint const & v3);
        void set_uint(std::string const & location, GLuint const & v0);
        void set_uint2(std::string const & location, GLuint const & v0, GLuint const & v1);
        void set_uint3(std::string const & location, GLuint const & v0, GLuint const & v1, GLuint const & v2);
        void set_uint4(std::string const & location, GLuint const & v0, GLuint const & v1, GLuint const & v2, GLuint const & v3);
        void set_floatv(std::string const & location, GLsizei const & count, GLfloat const * const & value);
        void set_floatv2(std::string const & location, GLsizei const & count, GLfloat const * const & value);
    	void set_floatv3(std::string const & location, GLsizei const & count, GLfloat const * const & value);
        void set_floatv4(std::string const & location, GLsizei const & count, GLfloat const * const & value);
        void set_intv(std::string const & location, GLsizei const & count, GLint const * const & value);
        void set_intv2(std::string const & location, GLsizei const & count, GLint const * const & value);
        void set_intv3(std::string const & location, GLsizei const & count, GLint const * const & value);
        void set_intv4(std::string const & location, GLsizei const & count, GLint const * const & value);
        void set_uintv(std::string const & location, GLsizei const & count, GLuint const * const & value);
        void set_uintv2(std::string const & location, GLsizei const & count, GLuint const * const & value);
        void set_uintv3(std::string const & location, GLsizei const & count, GLuint const * const & value);
        void set_uintv4(std::string const & location, GLsizei const & count, GLuint const * const & value);
        void set_mat_2x2_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_3x3_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_4x4_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_2x3_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_3x2_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_2x4_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_4x2_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_3x4_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);
        void set_mat_4x3_floatv(std::string const & location, GLsizei const & count, GLboolean const & transpose, GLfloat const * const & value);

    };
}