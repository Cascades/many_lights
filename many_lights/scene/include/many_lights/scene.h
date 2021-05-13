#include <vector>
#include <filesystem>

#include "many_lights/model.h"

namespace ml
{
	class Scene
	{
	private:
		ml::Model sphere_model;
		std::vector<ml::Model> models;

	public:
		Scene() : sphere_model("../assets/sphere/sphere.obj")
		{
		}

		template <typename T>
		Scene(std::initializer_list<T> model_paths) 
			requires (std::is_convertible_v<T, std::filesystem::path>) : sphere_model("../assets/sphere/sphere.obj")
		{
			for (auto const & model_path : model_paths)
			{
				add_model(model_path);
			}
		}

		~Scene() = default;

		std::vector<ml::Model> const& get_models() const
		{
			return models;
		}

		ml::Model const& get_sphere_model() const
		{
			return sphere_model;
		}

		void add_model (std::filesystem::path const& model_path)
		{
			models.emplace_back(model_path);
		}
	};
}