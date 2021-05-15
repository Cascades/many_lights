#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <filesystem>

namespace ml
{
    class Shader
    {
    public:
        Shader()
        {
            ID = UINT_MAX;
        }

        unsigned int ID;

        void file_to_string(std::filesystem::path const& shader_path, std::string& out_string);

        Shader(std::filesystem::path const& vert_path, std::filesystem::path const& frag_path);

        void use() const;

    private:
        void check_shader_compilation_errors(GLuint shader, GLuint type);

        void check_shader_program_linking_errors(GLuint shader);
    };
}