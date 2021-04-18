#pragma once

#include "many_lights/camera.h"
#include <memory>

namespace ml
{

	class Window
	{
		GLFWwindow* window;
		int width = 800;
		int height = 600;

		bool first_mouse = true;
		float last_x = width / 2.0f;
		float last_y = height / 2.0f;

		std::shared_ptr<ml::Camera> camera;

	public:
		Window(std::shared_ptr<ml::Camera> camera);
		~Window();

		int get_width() { return width; }
		int get_height() { return height; }

		void set_width(int const & new_width) { width = new_width; }
		void set_height(int const& new_height) { height = new_height; }

		void setup_viewport(int width, int height);

		GLFWwindow* get_window() const { return window; }

		void swap_buffers();

		void process_input(float const& delta_time);

	public:
		static void framebuffer_size_callback(GLFWwindow* window, int new_width, int new_height)
		{
			Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			win->set_width(new_width);
			win->set_height(new_height);
			glViewport(0, 0, new_width, new_height);
		}

		static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
		{
			Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			if (win->first_mouse)
			{
				win->last_x = xpos;
				win->last_y = ypos;
				win->first_mouse = false;
			}

			float xoffset = xpos - win->last_x;
			float yoffset = win->last_y - ypos;

			win->last_x = xpos;
			win->last_y = ypos;

			win->camera->ProcessMouseMovement(xoffset, yoffset);
		}

		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
		{
			Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			win->camera->ProcessMouseScroll(yoffset);
		}
	};

}