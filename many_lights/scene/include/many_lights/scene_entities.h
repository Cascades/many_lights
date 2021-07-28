#pragma once

#include <vector>
#include <filesystem>

#include "many_lights/model.h"

namespace ml
{
	class SceneEntities
	{
	private:
	//revert this hack
	public:
		std::vector<ml::Model> models;

	public:
		SceneEntities() = default;

		template <typename T>
		SceneEntities(std::initializer_list<T> model_paths)
			requires (std::is_convertible_v<T, std::filesystem::path>)
		{
			for (auto const& model_path : model_paths)
			{
				add_model(model_path);
			}
		}

		~SceneEntities() = default;

		std::vector<ml::Model>& get_models()
		{
			return models;
		}

		void add_model(std::filesystem::path const& model_path)
		{
			models.emplace_back(model_path);
		}
	};
}