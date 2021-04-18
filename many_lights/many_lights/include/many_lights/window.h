#pragma once

namespace ml
{

	class Window
	{
		GLFWwindow* window;

	public:
		Window();
		~Window();

		void setup_viewport(int width, int height);

		GLFWwindow* get_window() const { return window; }

		void swap_buffers();

		void process_input();

	public:
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
		}
	};

}