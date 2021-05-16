#pragma once

#include <many_lights/many_lights_algorithm.h>

namespace TestApplication
{
	class ForwardBlinnPhong final : public ml::ManyLightsAlgorithm
	{
	private:
		ml::Shader forward_blinn_phong;

	public:
		ForwardBlinnPhong();

		ForwardBlinnPhong(int const& width, int const& height);

		~ForwardBlinnPhong() override = default;

		void init(int const& width, int const& height) override;

		void adjust_size(int const& width, int const& height) override;

		void render(ml::Scene& scene) override;
	};
}