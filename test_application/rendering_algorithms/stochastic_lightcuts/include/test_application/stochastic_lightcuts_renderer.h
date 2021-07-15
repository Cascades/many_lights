#pragma once
#include <many_lights/many_lights_algorithm.h>
#include <many_lights/shader.h>

#include <vector>

#include "pbt/PBT.h"
#include "LightCut/LightCut.h"

#include <glm/gtc/type_ptr.hpp>

namespace TestApplication
{
	struct MiscVars
	{
		glm::vec4 viewPos;
		int32_t iFrame;
		int32_t lightcuts_size;
		int32_t tile_size;
		float random_tile_sample;
	};

	struct MortonVars {
		glm::vec4 max_bound;
		glm::vec4 min_bound;
		uint32_t num_leaves;
		uint32_t num_lights;
	};

	struct BitonicVars {
		uint32_t stage;
		uint32_t pass_num;
		uint32_t num_lights;
	};
	
	template <size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
	class StochasticLightcuts final : public ml::ManyLightsAlgorithm<max_lights>
	{
	public:
		StochasticLightcuts() : perfect_balanced_tree() {}
		
		ml::Shader geometry_pass_shader;
		ml::Shader light_pass_shader;

		ml::Shader generate_cut_shader;

		ml::Shader morton_code_shader;
		ml::Shader bitonic_sort_shader;
		ml::Shader internal_node_shader;

		unsigned int g_buffer;
		unsigned int g_position, g_normal, g_diff_spec, g_ambient;
		unsigned int lights_ssbo, lightcuts, miscVars, grid_cell_debug;
		unsigned int morton_vars, morton_ssbo, bitonic_vars;
		unsigned int depth_buffer;
		std::vector<unsigned int> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

		uint32_t frame_count = 0;
		int width = 800;
		int height = 600;

		uint32_t runtime_max_lights = max_lights;

		int32_t lightcuts_size;
		int generation_type = 0;

		float random_tiling = 1.0;
		bool random_tiling_bool = false;

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

		void ui(bool const& num_lights_changed, bool const& light_heights_changed, ml::Scene<max_lights> const& scene) override;
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
	morton_code_shader = std::move(ml::Shader("../assets/shaders/stochastic_lightcuts/morton_code.glsl"));
	bitonic_sort_shader = std::move(ml::Shader("../assets/shaders/stochastic_lightcuts/bitonic_sort.glsl"));
	internal_node_shader = std::move(ml::Shader("../assets/shaders/stochastic_lightcuts/build_internal_nodes.glsl"));

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

	glGenBuffers(1, &lights_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lights_ssbo);
	glNamedBufferStorage(lights_ssbo, perfect_balanced_tree.size_bytes(), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lights_ssbo);

	glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());

	glGenBuffers(1, &lightcuts);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightcuts);
	glNamedBufferStorage(lightcuts, sizeof(int) * ((800 / min_tile_size) + 1) * ((600 / min_tile_size) + 1) * max_lightcuts_size, NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightcuts);

	glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((800 / tile_size) + 1) * ((600 / tile_size) + 1) * lightcuts_size, lightcuts_array.data());

	glGenBuffers(1, &miscVars);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, miscVars);
	glNamedBufferStorage(miscVars, sizeof(MiscVars), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, miscVars);

	glGenBuffers(1, &morton_vars);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, morton_vars);
	glNamedBufferStorage(morton_vars, sizeof(MortonVars), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, morton_vars);

	glGenBuffers(1, &morton_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, morton_ssbo);
	glNamedBufferStorage(morton_ssbo, sizeof(uint32_t) * max_lights, NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, morton_ssbo);

	glGenBuffers(1, &bitonic_vars);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, bitonic_vars);
	glNamedBufferStorage(bitonic_vars, sizeof(BitonicVars), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, bitonic_vars);

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
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::ui(bool const& num_lights_changed, bool const& light_heights_changed, ml::Scene<max_lights> const & scene)
{
	if(num_lights_changed)
	{
		if (generation_type == 0)
		{
			glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
			glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((800 / tile_size) + 1) * ((600 / tile_size) + 1) * lightcuts_size, lightcuts_array.data());
		}
		else
		{
			std::array<typename PBT<float, float, max_lights>::array_data_type, std::bit_ceil(max_lights) * 2 - 1> d;

			d.fill(PBTNode<float, float>{});
			
			glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), d.data());

			glNamedBufferSubData(morton_vars, 0, sizeof(glm::vec4), glm::value_ptr(scene.lights->min_bounds));
			glNamedBufferSubData(morton_vars, sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(scene.lights->max_bounds));
		}

		std::vector<int32_t> grid_cell_debug_data;
		grid_cell_debug_data.assign(max_tile_size * max_tile_size * (max_lightcuts_size) * 3, -1);

		glNamedBufferStorage(grid_cell_debug, sizeof(int32_t) * max_tile_size * max_tile_size * (max_lightcuts_size) * 3, grid_cell_debug_data.data(), GL_DYNAMIC_STORAGE_BIT);
	}
	else if(light_heights_changed)
	{
		//glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
		//glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((800 / tile_size) + 1) * ((600 / tile_size) + 1) * lightcuts_size, lightcuts_array.data());
		
		//std::vector<int32_t> grid_cell_debug_data;
		//grid_cell_debug_data.assign(max_tile_size * max_tile_size * (max_lightcuts_size), -1);

		//glNamedBufferStorage(grid_cell_debug, sizeof(int32_t) * max_tile_size * max_tile_size * (max_lightcuts_size), grid_cell_debug_data.data(), GL_DYNAMIC_STORAGE_BIT);
		
	}
	
	ImGui::SliderInt("Lightcut Size", &lightcuts_size, 0, max_lightcuts_size);
	ImGui::SliderInt("Tile Size", &tile_size, min_tile_size, max_tile_size);

	if(ImGui::Checkbox("Random Tiling", &random_tiling_bool))
	{
		if (random_tiling == 1.0f)
		{
			random_tiling = 0.0f;
		}
		else
		{
			random_tiling = 1.0f;
		}
	}

	if(ImGui::RadioButton("CPU tree generation", &generation_type, 0))
	{
		std::cout << 0 << std::endl;
	}
	else if(ImGui::RadioButton("GPU tree generation", &generation_type, 1))
	{
		std::cout << 1 << std::endl;
		glNamedBufferSubData(morton_vars, 0, sizeof(glm::vec4), glm::value_ptr(scene.lights->min_bounds));
		glNamedBufferSubData(morton_vars, sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(scene.lights->max_bounds));

		glNamedBufferSubData(bitonic_vars, sizeof(uint32_t) * 3, sizeof(uint32_t), &runtime_max_lights);
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
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (generation_type == 0)
	{
		perfect_balanced_tree.regenerate(*scene.lights);
		//perfect_balanced_tree.print();
		glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
	}
	else
	{
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
		morton_code_shader.use(); // Compute shader program.
		glDispatchCompute(scene.lights->get_max_lights(), 1, 1);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		bitonic_sort_shader.use(); // Compute shader program.

		size_t bit_ceil_max = std::bit_ceil(max_lights) * 2 - 1;

		std::cout << "========== BEGIN ==========" << std::endl;
		std::cout << "bit_ceil_max: " << bit_ceil_max << std::endl;
		std::cout << "bit_ceil_max / 2: " << bit_ceil_max / 2 << std::endl;
		std::cout << "bit_ceil_max / 4: " << bit_ceil_max / 4 << std::endl;
		
		for (uint32_t stage = 0; stage < static_cast<uint32_t>(std::log2(static_cast<float>(bit_ceil_max))); stage++)
		{
			for (uint32_t pass_num = 0; pass_num <= stage; pass_num++)
			{
				std::cout << "stage: " << stage << std::endl;
				std::cout << "pass_num: " << pass_num << std::endl;
				
				// stage pass num_lights
				glNamedBufferSubData(bitonic_vars, 0, sizeof(uint32_t), &stage);
				glNamedBufferSubData(bitonic_vars, sizeof(uint32_t), sizeof(uint32_t), &pass_num);
				glNamedBufferSubData(bitonic_vars, sizeof(uint32_t) * 2, sizeof(uint32_t), &runtime_max_lights);

				// defo wrong
				glDispatchCompute(std::bit_ceil(max_lights) / 2, 1, 1);

				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				std::cout << "=====================" << std::endl;
			}
		}

		std::cout << "========== END ==========" << std::endl;

		internal_node_shader.use();

		glDispatchCompute((std::bit_ceil(max_lights) * 2 - 1) / 2, 1, 1);
	}

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	
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
	glNamedBufferSubData(miscVars, sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(int32_t) + sizeof(int32_t), sizeof(float), &random_tiling);
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