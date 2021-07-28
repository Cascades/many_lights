#pragma once

#include "many_lights/scene.h"

#include "string"

namespace ml
{
	template<size_t max_lights>
	class ManyLightsAlgorithm
	{
	public:
		ManyLightsAlgorithm() = default;
        ManyLightsAlgorithm(const ManyLightsAlgorithm& other) = default;
        ManyLightsAlgorithm(ManyLightsAlgorithm&& other) = default;
		ManyLightsAlgorithm& operator=(const ManyLightsAlgorithm& other) = default;
		ManyLightsAlgorithm& operator=(ManyLightsAlgorithm&& other) = default;
		virtual ~ManyLightsAlgorithm() = default;
		virtual void init([[maybe_unused]] int const& width, [[maybe_unused]] int const& height, [[maybe_unused]] ml::Scene<max_lights> const& scene) = 0;
		//virtual void init([[maybe_unused]] int const& width, [[maybe_unused]] int const& height) = 0;
		virtual void adjust_size([[maybe_unused]] int const& width, [[maybe_unused]] int const& height) = 0;
		virtual void render(ml::Scene<max_lights>& scene) = 0;

		virtual std::string get_name() const = 0;

		virtual void ui(bool const& num_lights_changed, bool const& light_heights_changed, std::shared_ptr<ml::Scene<max_lights>> scene) = 0;
	};
}