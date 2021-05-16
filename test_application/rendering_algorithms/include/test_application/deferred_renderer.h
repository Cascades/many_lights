#pragma once
#include <many_lights/many_lights_algorithm.h>
#include <many_lights/shader.h>

#include <vector>

namespace TestApplication
{
	class Deferred final : public ml::ManyLightsAlgorithm
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

		void init(int const& width, int const& height) override;

		void adjust_size(int const& width, int const& height) override;

		void render(ml::Scene& scene) override;
	};
}