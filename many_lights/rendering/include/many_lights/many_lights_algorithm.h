#pragma once

#include "many_lights/scene.h"

namespace ml
{
	class ManyLightsAlgorithm
	{
	public:
		ManyLightsAlgorithm() = default;
        ManyLightsAlgorithm(const ManyLightsAlgorithm& other) = default;
        ManyLightsAlgorithm(ManyLightsAlgorithm&& other) = default;
		ManyLightsAlgorithm& operator=(const ManyLightsAlgorithm& other) = default;
		ManyLightsAlgorithm& operator=(ManyLightsAlgorithm&& other) = default;
		virtual ~ManyLightsAlgorithm() = default;
		virtual void init([[maybe_unused]] int const& width, [[maybe_unused]] int const& height) = 0;
		virtual void adjust_size([[maybe_unused]] int const& width, [[maybe_unused]] int const& height) = 0;
		virtual void render(ml::Scene& scene) = 0;
	};
}