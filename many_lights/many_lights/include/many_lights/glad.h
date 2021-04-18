#pragma once

#include <glad/glad.h>
#include <glfw/glfw3.h>

namespace ml
{
    void intialise_glad()
    {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw "Failed to initialize GLAD";
        }
    }
}