#pragma once

#include "many_lights/scene.h"
#include "many_lights/camera.h"
#include "many_lights/shader.h"
#include "many_lights/scene_lights.h"
#include "many_lights/exceptions.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

class DeferredRenderData
{
public:
	ml::Shader geometry_pass_shader;
	ml::Shader light_pass_shader;
	unsigned int g_buffer;
	unsigned int g_position, g_normal, g_diff_spec, g_ambient;
	unsigned int depth_buffer;
	std::vector<unsigned int> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };


	unsigned int quadVAO;
	unsigned int quadVBO;

	void init(int const & width, int const & height)
	{
		geometry_pass_shader = std::move(ml::Shader("../assets/shaders/geometry_pass.vert", "../assets/shaders/geometry_pass.frag"));
		light_pass_shader = std::move(ml::Shader("../assets/shaders/light_pass.vert", "../assets/shaders/light_pass.frag"));

		glUniformBlockBinding(light_pass_shader.ID, glGetUniformBlockIndex(light_pass_shader.ID, "Lights"), 0);

		glGenFramebuffers(1, &g_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

		glGenTextures(1, &g_position);
		glGenTextures(1, &g_normal);
		glGenTextures(1, &g_diff_spec);
		glGenTextures(1, &g_ambient);
		glGenRenderbuffers(1, &depth_buffer);

		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

		glBindTexture(GL_TEXTURE_2D, g_position);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

		glBindTexture(GL_TEXTURE_2D, g_normal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);

		glBindTexture(GL_TEXTURE_2D, g_diff_spec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_diff_spec, 0);

		glBindTexture(GL_TEXTURE_2D, g_ambient);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_ambient, 0);

		glDrawBuffers(attachments.size(), attachments.data());

		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void adjust_size(int const& width, int const& height)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

		glBindTexture(GL_TEXTURE_2D, g_position);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

		glBindTexture(GL_TEXTURE_2D, g_normal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);

		glBindTexture(GL_TEXTURE_2D, g_diff_spec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_diff_spec, 0);

		glBindTexture(GL_TEXTURE_2D, g_ambient);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_ambient, 0);

		glDrawBuffers(attachments.size(), attachments.data());

		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};

class ForwardRenderData
{
public:
	ml::Shader forward_bling_phong;
	glm::mat4 model_matrix = glm::mat4(1.0f);
	glm::vec3 object_color = glm::vec3(1.0f, 1.0f, 1.0f);
	int model_matrix_location;
	int view_matrix_location;
	int projection_matrix_location;
	int view_position_location;
	int object_color_location;
	int num_lights_location;

	void init(int const& width, int const& height)
	{
		try
		{
			forward_bling_phong = ml::Shader("../assets/shaders/forward_blinn_phong.vert", "../assets/shaders/forward_blinn_phong.frag");
		}
		catch (ml::ShaderCompilationException const& e)
		{
			std::cout << e.what() << std::endl;
			throw e;
		}

		glUniformBlockBinding(forward_bling_phong.ID, glGetUniformBlockIndex(forward_bling_phong.ID, "Lights"), 0);

		model_matrix_location = glGetUniformLocation(forward_bling_phong.ID, "model");
		view_matrix_location = glGetUniformLocation(forward_bling_phong.ID, "view");
		projection_matrix_location = glGetUniformLocation(forward_bling_phong.ID, "projection");

		view_position_location = glGetUniformLocation(forward_bling_phong.ID, "viewPos");
		object_color_location = glGetUniformLocation(forward_bling_phong.ID, "objectColor");

		num_lights_location = glGetUniformLocation(forward_bling_phong.ID, "num_lights");
	}
};

namespace ml
{
	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer() = default;

		void render();

		void forward_render(ml::Scene& scene, ForwardRenderData& frd);

		void deferred_render(ml::Scene& scene, DeferredRenderData& drd);
	};
}