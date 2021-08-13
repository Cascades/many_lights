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
		glm::uvec2 screen_size;
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

		GLuint counter_val;

		bool full_range_test = false;
		bool first_range_test = false;

		unsigned int morton_time_query;
		unsigned int bitonic_sort_time_query;
		unsigned int internal_construction_time_query;
		unsigned int geometry_pass_time_query;
		unsigned int cut_generation_time_query;
		unsigned int shading_pass_time_query;
		uint64_t query_result;
		size_t iteration_counter = 0;

		unsigned int lighting_comp_atomic;
		unsigned int g_buffer;
		unsigned int g_position, g_normal, g_diff_spec, g_ambient;
		unsigned int lights_ssbo, lightcuts, miscVars, grid_cell_debug;
		unsigned int morton_vars, morton_ssbo, bitonic_vars;
		unsigned int depth_buffer;
		std::vector<unsigned int> attachments = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

		std::string time_extension;
		
		uint32_t frame_count = 0;
		int width = 1920;
		int height = 1080;

		uint32_t runtime_max_lights = max_lights;

		int32_t lightcuts_size;
		int generation_type = 1;

		float random_tiling = 1.0;
		bool random_tiling_bool = false;

		int32_t tile_size = 16;
		static constexpr int32_t min_tile_size = 2;

		std::vector<int> lightcuts_array;

		unsigned int quadVAO;
		unsigned int quadVBO;

		PBT<float, float, max_lights> perfect_balanced_tree;
		LightCut<float, float, max_lights> light_cut;

		void init(int const& width, int const& height, ml::Scene<max_lights> const & scene) override;

		void adjust_size(int const& width, int const& height) override;

		void render(ml::Scene<max_lights>& scene, GLFWwindow* window) override;

		std::string get_name() const override
		{
			return "Stochastic LightCuts";
		}

		void ui(bool const& num_lights_changed, bool const& light_heights_changed, std::shared_ptr<ml::Scene<max_lights>> scene) override;
	};
}

template<size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::init(int const& width, int const& height, ml::Scene<max_lights> const& scene)
{
	const std::time_t now = std::time(nullptr); // get the current time point
	const std::tm calendar_time = *std::localtime(std::addressof(now));

	std::cout << "              year: " << calendar_time.tm_year + 1900 << '\n'
		<< "    month (jan==1): " << calendar_time.tm_mon + 1 << '\n'
		<< "      day of month: " << calendar_time.tm_mday << '\n'
		<< "hour (24-hr clock): " << calendar_time.tm_hour << '\n'
		<< "            minute: " << calendar_time.tm_min << '\n'
		<< "            second: " << calendar_time.tm_sec << '\n';

	time_extension = std::to_string(calendar_time.tm_mday);
	time_extension += std::string("_");
	time_extension += std::to_string(calendar_time.tm_hour);
	time_extension += std::string("_");
	time_extension += std::to_string(calendar_time.tm_min);
	time_extension += std::string("_");
	time_extension += std::to_string(calendar_time.tm_sec);
	
	std::ofstream logging_file;
	logging_file.open(std::string("stochastic_log_") + time_extension + std::string(".csv"));
	logging_file << "Frame, Mode, Light Cut Size, Tile Size, Number of Lights, Morton Time (ms), Bitonic Time (ms), Contruction Time (ms), Geo Pass Time (ms), Cut Gen Time (ms), Shading Pass Time (ms), Total Time (ms), Possible FPS, Light Computations\n";
	logging_file.close();

	glGenQueries(1, &morton_time_query);
	glGenQueries(1, &bitonic_sort_time_query);
	glGenQueries(1, &internal_construction_time_query);
	glGenQueries(1, &geometry_pass_time_query);
	glGenQueries(1, &cut_generation_time_query);
	glGenQueries(1, &shading_pass_time_query);
	
	int workGroupSizes[3] = { 0 };
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workGroupSizes[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workGroupSizes[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workGroupSizes[2]);
	int workGroupCounts[3] = { 0 };
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workGroupCounts[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workGroupCounts[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workGroupCounts[2]);
	
	std::cout << "GL_MAX_COMPUTE_WORK_GROUP_SIZE: " << workGroupSizes[0] << "," << workGroupSizes[1] << "," << workGroupSizes[2] << std::endl;
	std::cout << "GL_MAX_COMPUTE_WORK_GROUP_COUNT: " << workGroupCounts[0] << "," << workGroupCounts[1] << "," << workGroupCounts[2] << std::endl;
	
	lightcuts_size = 10;

	lightcuts_array.resize(((width / min_tile_size) + 1) * ((height / min_tile_size) + 1) * max_lightcuts_size);
	
	std::fill(lightcuts_array.begin(), lightcuts_array.end(), -1);
	
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
	glNamedBufferStorage(lightcuts, sizeof(int) * ((width / min_tile_size) + 1) * ((height / min_tile_size) + 1) * max_lightcuts_size, NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lightcuts);

	glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((width / tile_size) + 1) * ((height / tile_size) + 1) * lightcuts_size, lightcuts_array.data());

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
	glNamedBufferStorage(morton_ssbo, sizeof(glm::uvec2) * std::bit_ceil(max_lights), NULL, GL_DYNAMIC_STORAGE_BIT);
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

	GLuint start_val = 0;

	glGenBuffers(1, &lighting_comp_atomic);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, lighting_comp_atomic);
	glNamedBufferData(lighting_comp_atomic, sizeof(GLuint), &start_val, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, lighting_comp_atomic);
	
	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
}

template<size_t max_lights, size_t max_lightcuts_size, int32_t max_tile_size>
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::ui(bool const& num_lights_changed, bool const& light_heights_changed, std::shared_ptr<ml::Scene<max_lights>> scene)
{
	if(num_lights_changed)
	{
		if (generation_type == 0)
		{
			glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
			glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((width / tile_size) + 1) * ((height / tile_size) + 1) * lightcuts_size, lightcuts_array.data());
		}
		else
		{
			glClearNamedBufferData(lights_ssbo, GL_RGBA32I, GL_RGBA32I, GL_INT, NULL);

			glNamedBufferSubData(morton_vars, 0, sizeof(glm::vec4), glm::value_ptr(scene->lights->min_bounds));
			glNamedBufferSubData(morton_vars, sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(scene->lights->max_bounds));
		}

		//std::vector<int32_t> grid_cell_debug_data;
		//grid_cell_debug_data.assign(max_tile_size * max_tile_size * (max_lightcuts_size) * 3, -1);

		//glNamedBufferStorage(grid_cell_debug, sizeof(int32_t) * max_tile_size * max_tile_size * (max_lightcuts_size) * 3, grid_cell_debug_data.data(), GL_DYNAMIC_STORAGE_BIT);

		int32_t minus_one = -1;
		
		glClearNamedBufferData(grid_cell_debug, GL_RGBA32I, GL_RGBA32I, GL_INT, &minus_one);
	}
	else if(light_heights_changed)
	{
		//glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());
		//glNamedBufferSubData(lightcuts, 0, sizeof(int) * ((width / tile_size) + 1) * ((height / tile_size) + 1) * lightcuts_size, lightcuts_array.data());
		
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
		glNamedBufferSubData(morton_vars, 0, sizeof(glm::vec4), glm::value_ptr(scene->lights->min_bounds));
		glNamedBufferSubData(morton_vars, sizeof(glm::vec4), sizeof(glm::vec4), glm::value_ptr(scene->lights->max_bounds));

		glNamedBufferSubData(bitonic_vars, sizeof(uint32_t) * 3, sizeof(uint32_t), &runtime_max_lights);
	}

	glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
	GLuint* userCounters = (GLuint*)glMapNamedBufferRange(lighting_comp_atomic,
		0,
		sizeof(GLuint),
		GL_MAP_READ_BIT
	);
	counter_val = *userCounters;
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

	if (ImGui::Button("Full Range Test"))
	{
		full_range_test = true;
		lightcuts_size = 0;
		tile_size = min_tile_size;
		scene->lights->set_num_lights(0);
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
void TestApplication::StochasticLightcuts<max_lights, max_lightcuts_size, max_tile_size>::render(ml::Scene<max_lights>& scene, GLFWwindow* window)
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, g_buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (generation_type == 0)
	{
		perfect_balanced_tree.regenerate(*scene.lights);
		//perfect_balanced_tree.print();
		glNamedBufferSubData(lights_ssbo, 0, perfect_balanced_tree.size_bytes(), perfect_balanced_tree.get_data());

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
	}
	else
	{
		glClearNamedBufferData(morton_ssbo, GL_RGBA32I, GL_RGBA32I, GL_INT, NULL);
		
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		
	    morton_code_shader.use(); // Compute shader program.
		glBeginQuery(GL_TIME_ELAPSED, morton_time_query);
		glDispatchCompute((std::bit_ceil(scene.lights->get_max_lights()) / 128) + 1, 1, 1);
		glEndQuery(GL_TIME_ELAPSED);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		bitonic_sort_shader.use(); // Compute shader program.

		size_t bit_ceil_max = std::bit_ceil(max_lights) * 2 - 1;

		//std::cout << "========== BEGIN ==========" << std::endl;
		//std::cout << "bit_ceil_max: " << bit_ceil_max << std::endl;
		//std::cout << "bit_ceil_max / 2: " << bit_ceil_max / 2 << std::endl;
		//std::cout << "bit_ceil_max / 4: " << bit_ceil_max / 4 << std::endl;
		
		glBeginQuery(GL_TIME_ELAPSED, bitonic_sort_time_query);
		for (uint32_t stage = 0; stage < static_cast<uint32_t>(std::log2(static_cast<float>(max_lights))); stage++)
		{
			for (uint32_t pass_num = 0; pass_num <= stage; pass_num++)
			{
				//std::cout << "stage: " << stage << std::endl;
				//std::cout << "pass_num: " << pass_num << std::endl;
				
				// stage pass num_lights
				glNamedBufferSubData(bitonic_vars, 0, sizeof(uint32_t), &stage);
				glNamedBufferSubData(bitonic_vars, sizeof(uint32_t), sizeof(uint32_t), &pass_num);
				glNamedBufferSubData(bitonic_vars, sizeof(uint32_t) * 2, sizeof(uint32_t), &runtime_max_lights);

				// defo wrong
				glDispatchCompute((std::bit_ceil(max_lights) / 2) / 1024, 1, 1);
				//glDispatchCompute(std::bit_ceil(max_lights) / 2, 1, 1);

				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

				//std::cout << "=====================" << std::endl;
			}
		}
		glEndQuery(GL_TIME_ELAPSED);

		internal_node_shader.use();

		uint32_t d = 1;
		
		glNamedBufferSubData(bitonic_vars, 2 * sizeof(uint32_t), sizeof(uint32_t), &d);
		
		glBeginQuery(GL_TIME_ELAPSED, internal_construction_time_query);
		for (uint32_t internal_level = static_cast<uint32_t>(std::log2(static_cast<float>(max_lights * 2 - 1))); internal_level > 0; --internal_level)
		{
			uint32_t actual_level = internal_level - 1;
			
			const uint32_t level_size = 1u << (actual_level);

			glNamedBufferSubData(bitonic_vars, 0, sizeof(uint32_t), &actual_level);
			glNamedBufferSubData(bitonic_vars, sizeof(uint32_t), sizeof(uint32_t), &level_size);

			glDispatchCompute((level_size / 64) + 1, 1, 1);

			//std::cout << "threads_to_dispatch: " << (level_size / 64) + 1 << std::endl;

			//std::cout << actual_level << " " << level_size << std::endl;

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			//glDispatchCompute(((std::bit_ceil(max_lights) * 2 - 1) / 2) / 64, 1, 1);
		}
		glEndQuery(GL_TIME_ELAPSED);
	}
	
	geometry_pass_shader.use();

	geometry_pass_shader.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	geometry_pass_shader.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(scene.camera->GetViewMatrix()));
	geometry_pass_shader.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(scene.camera->projection_matrix));
	
	glBeginQuery(GL_TIME_ELAPSED, geometry_pass_time_query);
	for (auto& model : scene.models->get_models())
	{
		model.draw(geometry_pass_shader);
	}
	glEndQuery(GL_TIME_ELAPSED);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	generate_cut_shader.use(); // Compute shader program.

	generate_cut_shader.set_int("g_position", 0);
	generate_cut_shader.set_int("g_normal", 1);
	generate_cut_shader.set_int("g_diff_spec", 2);
	generate_cut_shader.set_int("g_ambient", 3);

	glNamedBufferSubData(miscVars, 0, sizeof(glm::vec4), glm::value_ptr(glm::vec4(scene.camera->position, 1.0)));
	glNamedBufferSubData(miscVars, sizeof(glm::vec4), sizeof(glm::uvec2), glm::value_ptr(glm::uvec2(width, height)));
	glNamedBufferSubData(miscVars, sizeof(glm::uvec2) + sizeof(glm::vec4), sizeof(uint32_t), &frame_count);
	glNamedBufferSubData(miscVars, sizeof(glm::uvec2) + sizeof(glm::vec4) + sizeof(uint32_t), sizeof(uint32_t), &lightcuts_size);
	glNamedBufferSubData(miscVars, sizeof(glm::uvec2) + sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(int32_t), sizeof(int32_t), &tile_size);
	glNamedBufferSubData(miscVars, sizeof(glm::uvec2) + sizeof(glm::vec4) + sizeof(uint32_t) + sizeof(int32_t) + sizeof(int32_t), sizeof(float), &random_tiling);
	frame_count++;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_diff_spec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_ambient);

	glBeginQuery(GL_TIME_ELAPSED, cut_generation_time_query);
	glDispatchCompute(((width / tile_size + 1) + 31) / 32, ((height / tile_size + 1) + 31) / 32, 1);
	glEndQuery(GL_TIME_ELAPSED);
	
	/////////////////////////////////////////////////

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

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
	glBeginQuery(GL_TIME_ELAPSED, shading_pass_time_query);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEndQuery(GL_TIME_ELAPSED);
	glBindVertexArray(0);

	glMemoryBarrier(GL_QUERY_BUFFER_BARRIER_BIT);

	double total_time = 0.0;
	
	std::ofstream logging_file;
	logging_file.open(std::string("stochastic_log_") + time_extension + std::string(".csv"), std::ios::app);
	logging_file << iteration_counter << ",";
	if (generation_type == 0)
	{
		logging_file << "CPU" << ",";
	}
	else
	{
		logging_file << "GPU" << ",";
	}
	logging_file << lightcuts_size << ",";
	logging_file << tile_size << ",";
	logging_file << scene.lights->get_num_lights() << ",";
	glGetQueryObjectui64v(morton_time_query, GL_QUERY_RESULT, &query_result);
	logging_file << static_cast<double>(query_result) / 1000000.0 << ",";
	total_time += static_cast<double>(query_result) / 1000000.0;
	glGetQueryObjectui64v(bitonic_sort_time_query, GL_QUERY_RESULT, &query_result);
	logging_file << static_cast<double>(query_result) / 1000000.0 << ",";
	total_time += static_cast<double>(query_result) / 1000000.0;
	glGetQueryObjectui64v(internal_construction_time_query, GL_QUERY_RESULT, &query_result);
	logging_file << static_cast<double>(query_result) / 1000000.0 << ",";
	total_time += static_cast<double>(query_result) / 1000000.0;
	glGetQueryObjectui64v(geometry_pass_time_query, GL_QUERY_RESULT, &query_result);
	logging_file << static_cast<double>(query_result) / 1000000.0 << ",";
	total_time += static_cast<double>(query_result) / 1000000.0;
	glGetQueryObjectui64v(cut_generation_time_query, GL_QUERY_RESULT, &query_result);
	logging_file << static_cast<double>(query_result) / 1000000.0 << ",";
	total_time += static_cast<double>(query_result) / 1000000.0;
	glGetQueryObjectui64v(shading_pass_time_query, GL_QUERY_RESULT, &query_result);
	logging_file << static_cast<double>(query_result) / 1000000.0 << ",";
	total_time += static_cast<double>(query_result) / 1000000.0;
	logging_file << total_time << ",";
	logging_file << 1000.0 / total_time << ",";
	logging_file << counter_val << "\n";
	logging_file.close();

	glMemoryBarrier(GL_QUERY_BUFFER_BARRIER_BIT);

	iteration_counter++;

	if (full_range_test && first_range_test)
	{
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		std::string file_name_str = (std::string("stochastic_screenshot_") + std::to_string(tile_size) + std::string("_") + std::to_string(lightcuts_size - 1) + std::string(".png"));

		//saveImage(file_name_str.c_str(), window);

		if (lightcuts_size < max_lightcuts_size)
		{
			lightcuts_size++;
		}
		else
		{
			if(tile_size < max_tile_size)
			{
				tile_size++;
				lightcuts_size = 0;
			}
			else
			{
				if(scene.lights->get_num_lights() < max_lights)
				{
					scene.lights->set_num_lights(scene.lights->get_num_lights() + 1000);
					lightcuts_size = 0;
					tile_size = min_tile_size;
				}
				else
				{
					scene.lights->set_num_lights(0);
					lightcuts_size = 0;
					tile_size = min_tile_size;
					full_range_test = false;
					first_range_test = false;
				}
			}
		}

		std::cout << file_name_str << std::endl;
	}

	if (!first_range_test)
	{
		first_range_test = true;
	}
}