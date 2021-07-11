#pragma once 
#include <utility>
#include <vector>
#include <filesystem>
#include <memory>

#include "many_lights/model.h"
#include "many_lights/camera.h"
#include "many_lights/scene_entities.h"
#include "many_lights/scene_lights.h"

namespace ml
{
	template<size_t max_lights>
	struct Scene
	{
		Scene(std::shared_ptr<ml::Camera> camera, std::shared_ptr<ml::SceneEntities> models, std::shared_ptr<ml::SceneLights<max_lights>> lights) :
			camera(std::move(camera)),
			models(std::move(models)),
			lights(std::move(lights))
		{}
		std::shared_ptr<ml::Camera> camera;
		std::shared_ptr<ml::SceneEntities> models;
		std::shared_ptr<ml::SceneLights<max_lights>> lights;
	};
}