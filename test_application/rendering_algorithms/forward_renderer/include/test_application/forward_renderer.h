#pragma once

#include <many_lights/many_lights_algorithm.h>

namespace TestApplication
{
	class ForwardBlinnPhong final : public ml::ManyLightsAlgorithm
	{
	private:
		ml::Shader forward_blinn_phong;

	public:
		ForwardBlinnPhong() = default;
		
		ForwardBlinnPhong(ml::Scene const& scene);

		ForwardBlinnPhong(int const& width, int const& height, ml::Scene const& scene);

		~ForwardBlinnPhong() override = default;

		void init(int const& width, int const& height, ml::Scene const & scene) override;

		void adjust_size([[maybe_unused]] int const& width, [[maybe_unused]] int const& height) override;

		void render(ml::Scene& scene) override;

		std::string get_name() const override
		{
			return "Forward";
		}
	};
}