#pragma once

namespace ml
{
	class ManyLightsAlgorithm
	{
	private:
		virtual void init(int const& width, int const& height) = 0;

		virtual void adjust_size(int const& width, int const& height) = 0;

	public:
		ManyLightsAlgorithm() = default;
		virtual ManyLightsAlgorithm() = 0;

		virtual render(ml::Scene& scene, ForwardRenderData& frd) = 0
	};
}