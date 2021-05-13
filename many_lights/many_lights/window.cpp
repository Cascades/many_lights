#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "many_lights/window.h"
#include "many_lights/camera.h"

#include <memory>
#include <iostream>

void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam)
{
    std::cout << message << std::endl;
}

ml::Window::Window(std::shared_ptr<ml::Camera> camera)
{
    this->camera = camera;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow(800, 600, "MSc Project | George Tattersall | Many Lights", NULL, NULL);
    if (window == nullptr)
    {
        glfwTerminate();
        throw "Failed to create GLFW window";
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //glEnable(GL_DEBUG_OUTPUT);
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    //glDebugMessageCallback(debugMessage, NULL);

    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}

void ml::Window::setup_viewport(int new_width, int new_height)
{
    width = new_width;
    height = new_height;
    glViewport(0, 0, width, height);

    glfwSetWindowUserPointer(window, reinterpret_cast<void*>(this));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

void ml::Window::swap_buffers()
{
    glfwSwapBuffers(get_window());
}

void ml::Window::process_input(float const & delta_time)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !in_imgui)
        camera->ProcessKeyboard(ml::CameraMovement::FORWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !in_imgui)
        camera->ProcessKeyboard(ml::CameraMovement::BACKWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !in_imgui)
        camera->ProcessKeyboard(ml::CameraMovement::LEFT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !in_imgui)
        camera->ProcessKeyboard(ml::CameraMovement::RIGHT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !in_imgui)
    {
        camera->movenent_speed = 600.0f;
    }
    else if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && !in_imgui)
    {
        camera->movenent_speed = 200.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
    {
        in_imgui = !in_imgui;
        if (in_imgui)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

ml::Window::~Window()
{
    glfwTerminate();
}