#pragma once
#include <many_lights/many_lights_algorithm.h>
#include <many_lights/shader.h>

#include <vector>

#include "pbt/PBT.h"
#include "LightCut/LightCut.h"

#include <glm/gtc/type_ptr.hpp>

namespace TestApplication
{
	template <size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
	class StochasticLightcuts final : public ml::ManyLightsAlgorithm<max_lights>
	{
	public:
		StochasticLightcuts() : perfect_balanced_tree() {}
		
		ml::Shader geometry_pass_shader;
		ml::Shader light_pass_shader;

		ml::Shader generate_cut_shader;

		unsigned int g_buffer;
		unsigned int g_position, g_normal, g_diff_spec, g_ambient;
		unsigned int SSBO, lightcuts, miscVars, grid_cell_debug;
		unsigned int depth_buffer;
		std::vector<unsigned int> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

		uint32_t frame_count = 0;
		int width = 800;
		int height = 600;

		int32_t lightcuts_size;
		int generation_type = 0;

		int32_t tile_size = 16;
		static constexpr int32_t min_tile_size = 2;

		std::array<int, ((800/ min_tile_size) + 1) * ((600 / min_tile_size) + 1) * max_lightcuts_size> lightcuts_array;

		unsigned int quadVAO;
		unsigned int quadVBO;

		PBT<float, float, max_lights> perfect_balanced_tree;
		LightCut<float, float, max_lights> light_cut;

		void init(int const& width, int const& height, ml::Scene<max_lights> const & scene) override;

		void adjust_size(int const& width, int const& height) override;

		void render(ml::Scene<max_lights>& scene) override;

		std::string get_name() const override
		{
			return "Stochastic LightCuts";
		}

		void ui(bool const& num_lights_changed, bool const& light_heights_changed) override;
	};
}

template<size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::init(int const& width, int const& height, ml::Scene<max_lights> const& scene)
{
	lightcuts_size = 6;

	lightcuts_array.fill(-1);
	
	this->width = width;
	this->height = height;

	geometry_pass_shader = std::move(ml::Shader("../assets/shaders/geometry_pass.vert",
		"../assets/shaders/geometry_pass.frag"));
	light_pass_shader = std::move(ml::Shader("../assets/shaders/stochastic_lightcuts/light_pass.vert",
		"../assets/shaders/stochastic_lightcuts/light_pass.frag"));
	generate_cut_shader = std::move(ml::Shader("../assets/shaders/stochastic_lightcuts/generate_cuts.glsl"));

	glUniformBlockBinding(*light_pass_shader.id, glGetUniformBlockIndex(*light_pass_shader.id, "Lights"), 0);

	glGenFramebuffers(1, &g_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	glGenTextures(1, &g_position);
	glGenTextures(1, &g_normal);
	glGenTextures(1, &g_diff_spec);
	glGenTextures(1, &g_ambient);
	glGenRenderbuffers(1, &depth_buffer);

	float quadVertices[] = {
		// positions        // texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
	// setup plane VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	glBindTexture(GL_TEXTURE_2D, g_position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

	glBindTexture(GL_TEXTURE_2D, g_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);

	glBindTexture(GL_TEXTURE_2D, g_diff_spec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_diff_spec, 0);

	glBindTexture(GL_TEXTURE_2D, g_ambient);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_ambient, 0);

	glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());

	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	perfect_balanced_tree.regenerate(*scene.lights);
	//perfect_balanced_tree.print();

	glGenBuffers(1, &SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	glNamedBufferStorage(SSBO, perfect_balanced_tree.size_bytes(), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

	glNamedBufferSubData(SSBO, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());

	glGenBuffers(1, &lightcuts);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightcuts);
	glNamedBufferStorage(lightcuts, sizeof(int) * ((800 / min_tile_size) + 1) * ((600 / min_tile_size) + 1) * max_lightcuts_size, NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightcuts);

	glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((800 / tile_size) + 1) * ((600 / tile_size) + 1) * lightcuts_size, lightcuts_array.data());

	glGenBuffers(1, &miscVars);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, miscVars);
	glNamedBufferStorage(miscVars, sizeof(glm::vec4) + sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, miscVars);

	glGenBuffers(1, &grid_cell_debug);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, grid_cell_debug);

	std::vector<int32_t> grid_cell_debug_data;
	grid_cell_debug_data.assign(min_tile_size * min_tile_size * max_lightcuts_size * 3, -1);
	
	glNamedBufferStorage(grid_cell_debug, sizeof(int32_t) * min_tile_size * min_tile_size * max_lightcuts_size * 3, grid_cell_debug_data.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, grid_cell_debug);

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
}

template<size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::ui(bool const& num_lights_changed, bool const& light_heights_changed)
{
	if(num_lights_changed)
	{
		glNamedBufferSubData(SSBO, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
		glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((800 / tile_size) + 1) * ((600 / tile_size) + 1) * lightcuts_size, lightcuts_array.data());

		std::vector<int32_t> grid_cell_debug_data;
		grid_cell_debug_data.assign(max_tile_size * max_tile_size * (max_lightcuts_size) * 3, -1);

		glNamedBufferStorage(grid_cell_debug, sizeof(int32_t) * max_tile_size * max_tile_size * (max_lightcuts_size) * 3, grid_cell_debug_data.data(), GL_DYNAMIC_STORAGE_BIT);
	}
	else if(light_heights_changed)
	{
		glNamedBufferSubData(SSBO, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
		glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((800 / tile_size) + 1) * ((600 / tile_size) + 1) * lightcuts_size, lightcuts_array.data());
		
		std::vector<int32_t> grid_cell_debug_data;
		grid_cell_debug_data.assign(max_tile_size * max_tile_size * (max_lightcuts_size), -1);

		glNamedBufferStorage(grid_cell_debug, sizeof(int32_t) * max_tile_size * max_tile_size * (max_lightcuts_size), grid_cell_debug_data.data(), GL_DYNAMIC_STORAGE_BIT);
		
	}
	
	ImGui::SliderInt("Lightcut Size", &lightcuts_size, 0, max_lightcuts_size);
	ImGui::SliderInt("Tile Size", &tile_size, min_tile_size, max_tile_size);

	if(ImGui::RadioButton("CPU tree generation", &generation_type, 0))
	{
		std::cout << 0 << std::endl;
	}
	else if(ImGui::RadioButton("GPU tree generation", &generation_type, 1))
	{
		std::cout << 1 << std::endl;
	}
}

template<size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::adjust_size(int const& width, int const& height)
{
	this->width = width;
	this->height = height;

	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);

	glBindTexture(GL_TEXTURE_2D, g_position);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_position, 0);

	glBindTexture(GL_TEXTURE_2D, g_normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_normal, 0);

	glBindTexture(GL_TEXTURE_2D, g_diff_spec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_diff_spec, 0);

	glBindTexture(GL_TEXTURE_2D, g_ambient);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_ambient, 0);

	glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());

	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

template<size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::render(ml::Scene<max_lights>& scene)
{
	perfect_balanced_tree.regenerate(*scene.lights);
	//perfect_balanced_tree.print();
	glNamedBufferSubData(SSBO, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
	
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	geometry_pass_shader.use();

	geometry_pass_shader.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	geometry_pass_shader.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(scene.camera->GetViewMatrix()));
	geometry_pass_shader.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(scene.camera->projection_matrix));

	for (auto& model : scene.models->get_models())
	{
		model.draw(geometry_pass_shader);
	}

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	generate_cut_shader.use(); // Compute shader program.

	generate_cut_shader.set_int("g_position", 0);
	generate_cut_shader.set_int("g_normal", 1);
	generate_cut_shader.set_int("g_diff_spec", 2);
	generate_cut_shader.set_int("g_ambient", 3);

	glNamedBufferSubData(miscVars, 0, sizeof(glm::vec4), glm::value_ptr(glm::vec4(scene.camera->position, 1.0)));
	glNamedBufferSubData(miscVars, sizeof(glm::vec4), sizeof(uint32_t), &frame_count);
	glNamedBufferSubData(miscVars, sizeof(glm::vec4) + sizeof(uint32_t), sizeof(uint32_t), &lightcuts_size);
	glNamedBufferSubData(miscVars, sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(int32_t), sizeof(int32_t), &tile_size);
	frame_count++;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_diff_spec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_ambient);

	glDispatchCompute(width / tile_size + 1, height / tile_size + 1, 1);

	/////////////////////////////////////////////////

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	light_pass_shader.use();

	//light_pass_shader.set_floatv3("viewPos", 1, glm::value_ptr(scene.camera->position));

	light_pass_shader.set_int("g_position", 0);
	light_pass_shader.set_int("g_normal", 1);
	light_pass_shader.set_int("g_diff_spec", 2);
	light_pass_shader.set_int("g_ambient", 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_diff_spec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_ambient);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}