#pragma once

#include <many_lights/many_lights_algorithm.h>

namespace TestApplication
{
	class ForwardBlinnPhong final : public ml::ManyLightsAlgorithm
	{
	private:
		ml::Shader forward_blinn_phong;
		glm::mat4 model_matrix = glm::mat4(1.0f);
		glm::vec3 object_color = glm::vec3(1.0f, 1.0f, 1.0f);

	public:
		ForwardBlinnPhong();

		ForwardBlinnPhong(int const& width, int const& height);

		~ForwardBlinnPhong() override = default;

		void init(int const& width, int const& height) override;

		void adjust_size(int const& width, int const& height) override;

		void render(ml::Scene& scene) override;
	};
}