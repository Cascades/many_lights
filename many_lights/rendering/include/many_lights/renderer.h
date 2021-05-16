#pragma once

namespace ml
{
	class Renderer
	{
	public:
		Renderer() = default;
		~Renderer() = default;
		Renderer(Renderer const& other) = default;
		Renderer& operator=(Renderer const& other) = default;
		Renderer(Renderer&& other) = default;
		Renderer& operator=(Renderer&& other) = default;

		void render();
	};
}