#pragma once

#include <many_lights/many_lights_algorithm.h>

#include <many_lights/exceptions.h>

#include "glm/gtc/type_ptr.hpp"

namespace TestApplication
{
	template<size_t max_lights>
	class VPLDebug final : public ml::ManyLightsAlgorithm<max_lights>
	{
	private:
		ml::Shader forward_blinn_phong;

		bool vpls_on = false;

	public:
		VPLDebug() = default;
		
		VPLDebug(ml::Scene<max_lights> const& scene);

		VPLDebug(int const& width, int const& height, ml::Scene<max_lights> const& scene);

		~VPLDebug() override = default;

		void init(int const& width, int const& height, ml::Scene<max_lights> const & scene) override;

		void adjust_size([[maybe_unused]] int const& width, [[maybe_unused]] int const& height) override;

		void render(ml::Scene<max_lights>& scene, GLFWwindow* window) override;

		std::string get_name() const override
		{
			return "VPL Debug";
		}

		void ui(bool const& num_lights_changed, bool const& light_heights_changed, std::shared_ptr<ml::Scene<max_lights>> scene) override
		{
			ImGui::Checkbox("VPLs On", &vpls_on);
		}
	};

	template<size_t max_lights>
	VPLDebug<max_lights>::VPLDebug(ml::Scene<max_lights> const& scene)
	{
		VPLDebug::init(800, 600, scene);
	}

	template<size_t max_lights>
	VPLDebug<max_lights>::VPLDebug(int const& width, int const& height, ml::Scene<max_lights> const& scene)
	{
		VPLDebug::init(width, height, scene);
	}

	template<size_t max_lights>
	void VPLDebug<max_lights>::init(int const& width, int const& height, ml::Scene<max_lights> const& scene)
	{
		try
		{
			forward_blinn_phong = ml::Shader("../assets/shaders/forward_blinn_phong.vert", "../assets/shaders/vpl_debug_no_lights.frag");
		}
		catch (ml::ShaderCompilationException const& e)
		{
			std::cout << e.what() << std::endl;
			throw e;
		}

		glUniformBlockBinding(*forward_blinn_phong.id, glGetUniformBlockIndex(*forward_blinn_phong.id, "Lights"), 0);
	}

	template<size_t max_lights>
	void VPLDebug<max_lights>::adjust_size([[maybe_unused]] int const& width, [[maybe_unused]] int const& height)
	{
		return;
	}

	template<size_t max_lights>
	void VPLDebug<max_lights>::render(ml::Scene<max_lights>& scene, GLFWwindow* window)
	{
		glEnable(GL_DEPTH_TEST);

		glm::vec3 view_position = scene.camera->position;
		glm::mat4 view_matrix = scene.camera->GetViewMatrix();
		glm::mat4 projection_matrix = scene.camera->projection_matrix;

		forward_blinn_phong.use();

		forward_blinn_phong.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		forward_blinn_phong.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(view_matrix));
		forward_blinn_phong.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(projection_matrix));

		for (auto& model : scene.models->get_models())
		{
			model.draw(forward_blinn_phong);
		}

		if (vpls_on)
		{
			scene.lights->sphereShader.use();

			scene.lights->sphereShader.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			scene.lights->sphereShader.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(scene.camera->GetViewMatrix()));
			scene.lights->sphereShader.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(scene.camera->projection_matrix));

			glBindVertexArray(scene.lights->sphereVAO);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene->lights->sphereEBO);
			glDrawElementsInstanced(GL_TRIANGLES, scene.lights->sphere.get_meshes()[0].indices.size(), GL_UNSIGNED_INT, nullptr, scene.lights->get_num_lights());

			glBindVertexArray(0);
		}
	}
}