#pragma once
#include <many_lights/many_lights_algorithm.h>
#include <many_lights/shader.h>

#include <vector>

#include "glm/gtc/type_ptr.hpp"

namespace TestApplication
{
	template<size_t max_lights>
	class Deferred final : public ml::ManyLightsAlgorithm<max_lights>
	{
	public:
		ml::Shader geometry_pass_shader;
		ml::Shader light_pass_shader;
		unsigned int lighting_comp_atomic;
		unsigned int g_buffer;
		unsigned int g_position, g_normal, g_diff_spec, g_ambient;
		unsigned int depth_buffer;
		std::vector<unsigned int> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };


		unsigned int quadVAO;
		unsigned int quadVBO;

		int32_t render_mode = 0;

		void init(int const& width, int const& height, ml::Scene<max_lights> const& scene) override;

		void adjust_size(int const& width, int const& height) override;

		void render(ml::Scene<max_lights>& scene) override;

		std::string get_name() const override
		{
			return "Deferred";
		}

		void ui(bool const& num_lights_changed, bool const& light_heights_changed, std::shared_ptr<ml::Scene<max_lights>> scene) override
		{
			ImGui::RadioButton("Position", &render_mode, 0);
			ImGui::RadioButton("Normal", &render_mode, 1);
			ImGui::RadioButton("Diffuse", &render_mode, 2);
			ImGui::RadioButton("Specular", &render_mode, 3);
			ImGui::RadioButton("Ambient", &render_mode, 4);
			ImGui::RadioButton("Composed", &render_mode, 5);
			glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
			GLuint* userCounters = (GLuint*)glMapNamedBufferRange(lighting_comp_atomic,
				0,
				sizeof(GLuint),
				GL_MAP_READ_BIT
			);
			GLuint counter_val = *userCounters;
			glUnmapNamedBuffer(lighting_comp_atomic);

			userCounters = (GLuint*)glMapNamedBufferRange(lighting_comp_atomic,
				0,
				sizeof(GLuint),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
			);
			memset(userCounters, 0, sizeof(GLuint));
			glUnmapNamedBuffer(lighting_comp_atomic);

			ImGui::Text("lighting computations = %d", counter_val);
			glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
		}
	};

	template<size_t max_lights>
	void Deferred<max_lights>::init(int const& width, int const& height, ml::Scene<max_lights> const& scene)
	{
		geometry_pass_shader = std::move(ml::Shader("../assets/shaders/geometry_pass.vert",
			"../assets/shaders/geometry_pass.frag"));
		light_pass_shader = std::move(ml::Shader("../assets/shaders/light_pass.vert",
			"../assets/shaders/light_pass.frag"));

		glUniformBlockBinding(*light_pass_shader.id, glGetUniformBlockIndex(*light_pass_shader.id, "Lights"), 0);

		glGenFramebuffers(1, &g_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

		glGenTextures(1, &g_position);
		glGenTextures(1, &g_normal);
		glGenTextures(1, &g_diff_spec);
		glGenTextures(1, &g_ambient);
		glGenRenderbuffers(1, &depth_buffer);

		GLuint start_val = 0;

		glGenBuffers(1, &lighting_comp_atomic);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, lighting_comp_atomic);
		glNamedBufferData(lighting_comp_atomic, sizeof(GLuint), &start_val, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, lighting_comp_atomic);

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

		glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());

		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	template<size_t max_lights>
	void Deferred<max_lights>::adjust_size(int const& width, int const& height)
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

		glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());

		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	template<size_t max_lights>
	void Deferred<max_lights>::render(ml::Scene<max_lights>& scene)
	{
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		geometry_pass_shader.use();

		geometry_pass_shader.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		geometry_pass_shader.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(scene.camera->GetViewMatrix()));
		geometry_pass_shader.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(scene.camera->projection_matrix));

		for (auto& model : scene.models->get_models())
		{
			model.draw(geometry_pass_shader);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		/////////////////////////////////////////////////

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		light_pass_shader.use();

		light_pass_shader.set_floatv3("viewPos", 1, glm::value_ptr(scene.camera->position));
		light_pass_shader.set_uint("num_lights", scene.lights->get_num_lights());
		light_pass_shader.set_int("render_mode", render_mode);

		light_pass_shader.set_int("g_position", 0);
		light_pass_shader.set_int("g_normal", 1);
		light_pass_shader.set_int("g_diff_spec", 2);
		light_pass_shader.set_int("g_ambient", 3);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_position);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g_normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, g_diff_spec);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, g_ambient);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
}