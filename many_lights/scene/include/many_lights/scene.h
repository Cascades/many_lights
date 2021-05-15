#pragma once 
#include <vector>
#include <filesystem>
#include <memory>

#include "many_lights/model.h"
#include "many_lights/camera.h"
#include "many_lights/scene_entities.h"
#include "many_lights/scene_lights.h"

namespace ml
{
	struct Scene
	{
		Scene(std::shared_ptr<ml::Camera> camera, std::shared_ptr<ml::SceneEntities> models, std::shared_ptr<ml::SceneLights> lights) :
			camera(camera),
			models(models),
			lights(lights)
		{}
		std::shared_ptr<ml::Camera> camera;
		std::shared_ptr<ml::SceneEntities> models;
		std::shared_ptr<ml::SceneLights> lights;
	};
}