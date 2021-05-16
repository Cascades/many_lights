#pragma once

#include "many_lights/scene.h"

namespace ml
{
	class ManyLightsAlgorithm
	{
	public:
		ManyLightsAlgorithm() = default;
		virtual ~ManyLightsAlgorithm() = default;
		virtual void init(int const& width, int const& height) = 0;
		virtual void adjust_size(int const& width, int const& height) = 0;
		virtual void render(ml::Scene& scene) = 0;
	};
}