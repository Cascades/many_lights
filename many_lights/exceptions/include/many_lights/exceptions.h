#pragma once
#include <exception>

namespace ml
{
    class ShaderCompilationException : public std::exception
    {
    public:
        int shader_type;
        GLchar infoLog[1024];

        ShaderCompilationException() = default;
        virtual ~ShaderCompilationException() throw() {};
        ShaderCompilationException(int const& shader_type) : shader_type(shader_type) {}
        const char* what() const noexcept override;
    };

    class ShaderProgramLinkingException : public std::exception
    {
    public:
        GLchar infoLog[1024];

        virtual ~ShaderProgramLinkingException() throw() {};
        const char* what() const noexcept override;
    };
}