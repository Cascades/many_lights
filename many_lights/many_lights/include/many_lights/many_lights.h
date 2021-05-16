#pragma once
#include <memory>

#include "many_lights/camera.h"
#include "many_lights/window.h"
#include "many_lights/renderer.h"
#include "many_lights/many_lights_algorithm.h"

namespace ml
{
	class ManyLights
	{
	public:
		std::shared_ptr<ml::Camera> camera;
		ml::Window window;

		std::shared_ptr<ml::SceneEntities> models;
		std::shared_ptr<ml::SceneLights> lights;
		std::shared_ptr<ml::Scene> scene;

		std::unique_ptr<ml::ManyLightsAlgorithm> algorithm;

		ml::Renderer renderer;

		ManyLights();

		void add_model(std::filesystem::path const& model) const;

		void set_lights(unsigned int const& max_lights, int const& num_lights, float const& lights_height) const;

		template <class T, class... ArgTypes>
		void set_algorithm(ArgTypes&&... args);

		void run();
	};

	template <class T, class ... ArgTypes>
	void ManyLights::set_algorithm(ArgTypes&&... args)
	{
		algorithm = std::make_unique<T>(args...);
		algorithm->init(window.get_width(), window.get_height());
	}
}
