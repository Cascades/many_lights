#pragma once

#include <glad/glad.h>

namespace ml
{
	inline void initalise_glad()
    {
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }
    }
}