#pragma once

#include <many_lights/many_lights_algorithm.h>

#include <many_lights/exceptions.h>

#include "glm/gtc/type_ptr.hpp"

namespace TestApplication
{
	template<size_t max_lights>
	class ForwardBlinnPhong final : public ml::ManyLightsAlgorithm<max_lights>
	{
	private:
		ml::Shader forward_blinn_phong;

		unsigned int lighting_comp_atomic;
		unsigned int lighting_comp_atomic_back;
		void* lighting_comp_atomic_map;

	public:
		ForwardBlinnPhong() = default;
		
		ForwardBlinnPhong(ml::Scene<max_lights> const& scene);

		ForwardBlinnPhong(int const& width, int const& height, ml::Scene<max_lights> const& scene);

		~ForwardBlinnPhong() override
		{
			glUnmapNamedBuffer(lighting_comp_atomic);
		}

		void init(int const& width, int const& height, ml::Scene<max_lights> const & scene) override;

		void adjust_size([[maybe_unused]] int const& width, [[maybe_unused]] int const& height) override;

		void render(ml::Scene<max_lights>& scene, GLFWwindow* window) override;

		std::string get_name() const override
		{
			return "Forward";
		}

		void ui(bool const& num_lights_changed, bool const& light_heights_changed, std::shared_ptr<ml::Scene<max_lights>> scene) override
		{
			glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
			GLuint* userCounters = (GLuint*)glMapNamedBufferRange(lighting_comp_atomic,
				0,
				sizeof(GLuint),
				GL_MAP_READ_BIT
			);
			GLuint counter_val = *userCounters;
			glUnmapNamedBuffer(lighting_comp_atomic);

			userCounters = (GLuint*)glMapNamedBufferRange(lighting_comp_atomic,
				0,
				sizeof(GLuint),
				GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
			);
			memset(userCounters, 0, sizeof(GLuint));
			glUnmapNamedBuffer(lighting_comp_atomic);
			
			ImGui::Text("lighting computations = %d", counter_val);
			glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
		}
	};

	template<size_t max_lights>
	ForwardBlinnPhong<max_lights>::ForwardBlinnPhong(ml::Scene<max_lights> const& scene)
	{
		ForwardBlinnPhong::init(800, 600, scene);
	}

	template<size_t max_lights>
	ForwardBlinnPhong<max_lights>::ForwardBlinnPhong(int const& width, int const& height, ml::Scene<max_lights> const& scene)
	{
		ForwardBlinnPhong::init(width, height, scene);
	}

	template<size_t max_lights>
	void ForwardBlinnPhong<max_lights>::init(int const& width, int const& height, ml::Scene<max_lights> const& scene)
	{
		try
		{
			forward_blinn_phong = ml::Shader("../assets/shaders/forward_blinn_phong.vert", "../assets/shaders/forward_blinn_phong.frag");
		}
		catch (ml::ShaderCompilationException const& e)
		{
			std::cout << e.what() << std::endl;
			throw e;
		}

		glUniformBlockBinding(*forward_blinn_phong.id, glGetUniformBlockIndex(*forward_blinn_phong.id, "Lights"), 0);

		GLuint start_val = 0;
		
		glGenBuffers(1, &lighting_comp_atomic);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, lighting_comp_atomic);
		glNamedBufferData(lighting_comp_atomic, sizeof(GLuint), &start_val, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, lighting_comp_atomic);
	}

	template<size_t max_lights>
	void ForwardBlinnPhong<max_lights>::adjust_size([[maybe_unused]] int const& width, [[maybe_unused]] int const& height)
	{
		return;
	}

	template<size_t max_lights>
	void ForwardBlinnPhong<max_lights>::render(ml::Scene<max_lights>& scene, GLFWwindow* window)
	{
		glEnable(GL_DEPTH_TEST);

		glm::vec3 view_position = scene.camera->position;
		glm::mat4 view_matrix = scene.camera->GetViewMatrix();
		glm::mat4 projection_matrix = scene.camera->projection_matrix;

		forward_blinn_phong.use();

		forward_blinn_phong.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		forward_blinn_phong.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(view_matrix));
		forward_blinn_phong.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(projection_matrix));

		forward_blinn_phong.set_uintv("num_lights", 1, &scene.lights->get_num_lights());

		forward_blinn_phong.set_floatv3("viewPos", 1, glm::value_ptr(view_position));

		for (auto& model : scene.models->get_models())
		{
			model.draw(forward_blinn_phong);
		}
	}
}