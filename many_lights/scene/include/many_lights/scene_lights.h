#pragma once
#include <array>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "many_lights/colors.h"
#include "many_lights/buffer.h"

namespace ml
{
	template<size_t max_lights>
    class SceneLights
    {
    private:
        struct Light
        {
            glm::vec4 position;
            glm::vec4 color;
        };

        std::array<Light, max_lights> lights;
        std::array<glm::mat4, max_lights> light_model_matrices;
        std::array<glm::vec4, max_lights> light_model_positions;
        std::array<glm::vec4, max_lights> light_colors;
        std::array<glm::vec3, max_lights> light_rand_offsets;
    	
        uint32_t num_lights = 70;
        float lights_height = 3.0f;

        ml::Buffer<GL_SHADER_STORAGE_BUFFER> ubo_light_block;

        void calculate_light_positions()
        {
            //num_lights = 1;
        	
            for (uint32_t light_index = 0; light_index < num_lights; ++light_index)
            {
                float x_pos = -1000.0f + ((light_index % 10) * (2000.0f / static_cast<float>(std::min(num_lights, 10u)))) + light_rand_offsets[light_index].x;
                float y_pos = lights_height + light_rand_offsets[light_index].y;
                float max_rows = std::max(static_cast<float>((static_cast<int64_t>(num_lights) / 10) - 1), 0.0f);
                float curr_row = static_cast<float>((light_index / 10));
                float z_pos = static_cast<float>(-((70 * max_rows) / 2) + (curr_row * 70) - 37) + light_rand_offsets[light_index].z;

                // root bounding box calculation
                x_bounds[1] = glm::max(x_pos, x_bounds[1]);
                x_bounds[0] = glm::min(x_pos, x_bounds[0]);
                y_bounds[1] = glm::max(y_pos, y_bounds[1]);
                y_bounds[0] = glm::min(y_pos, y_bounds[0]);
                z_bounds[1] = glm::max(z_pos, z_bounds[1]);
                z_bounds[0] = glm::min(z_pos, z_bounds[0]);
            	
                lights[light_index].position = glm::vec4(x_pos, y_pos, z_pos, 1.0);
                lights[light_index].color = glm::vec4(ml::utils::hsv_to_rgb(glm::vec3(light_index * (1.0f / num_lights), 1.0, 1.0)), 1.0);
                light_colors[light_index] = glm::vec4(ml::utils::hsv_to_rgb(glm::vec3(light_index * (1.0f / num_lights), 1.0, 1.0)), 1.0);
                light_model_matrices[light_index] = glm::mat4(1.0f);
                light_model_matrices[light_index] = glm::translate(light_model_matrices[light_index], glm::vec3(x_pos, y_pos, z_pos));
                light_model_positions[light_index] = glm::vec4(x_pos, y_pos, z_pos, 1.0);
            }
        }

    public:
        ml::Model sphere;
        unsigned int sphereVAO, sphereVBO, sphereEBO, sphereMatrixVBO, sphereColorVBO;
        ml::Shader sphereShader;
		
        glm::vec2 x_bounds = glm::vec2(0.0f);
        glm::vec2 y_bounds = glm::vec2(0.0f);
        glm::vec2 z_bounds = glm::vec2(0.0f);
		
        SceneLights() :
            lights(),
            light_model_matrices(),
            light_colors(),
            light_rand_offsets(),
            sphere("../assets/sphere/sphere.obj"),
            sphereShader("../assets/shaders/debug_spheres/debug_spheres.vert", "../assets/shaders/debug_spheres/debug_spheres.frag")
        {
            std::default_random_engine e;
            std::uniform_real_distribution<> dis_x(0.0,  5.0);
            std::uniform_real_distribution<> dis_y(0.0, 250.0);
            std::uniform_real_distribution<> dis_z(0.0, 5.0);
        	
            for (uint32_t light_index = 0; light_index < max_lights; ++light_index)
            {
                //light_rand_offsets[light_index] = glm::vec3(0.0f);
                light_rand_offsets[light_index] = glm::vec3(dis_x(e), dis_y(e), dis_z(e));
            }
        };
    	
        void initialise()
        {
            //lights.assign(max_lights, Light{});
            //light_colors.assign(max_lights, glm::vec4(0.0f));
            //light_model_matrices.assign(max_lights, glm::mat4(0.0));

            calculate_light_positions();

            ubo_light_block.buffer_data(size_bytes(), GL_DYNAMIC_DRAW);
            ubo_light_block.bind_buffer_range(static_cast<size_t>(0), static_cast<size_t>(0), size_bytes());        	
            ubo_light_block.buffer_sub_data(0, size_bytes(), data());

            ubo_light_block.bind_buffer_base(4);

            std::cout << "6" << glGetError() << std::endl;

            glGenBuffers(1, &sphereMatrixVBO);
            glBindBuffer(GL_ARRAY_BUFFER, sphereMatrixVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * max_lights, &light_model_positions[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &sphereColorVBO);
            glBindBuffer(GL_ARRAY_BUFFER, sphereColorVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * max_lights, &light_colors[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenVertexArrays(1, &sphereVAO);
            glGenBuffers(1, &sphereVBO);
            glGenBuffers(1, &sphereEBO);

            glBindVertexArray(sphereVAO);

            glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * sphere.get_meshes()[0].vertices.size(), sphere.get_meshes()[0].vertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.get_meshes()[0].indices.size() * sizeof(unsigned int),
                &sphere.get_meshes()[0].indices[0], GL_STATIC_DRAW);

            std::cout << "5" << glGetError() << std::endl;

            // vertex positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            // vertex normals
            //glEnableVertexAttribArray(1);
            //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            // vertex texture coords
            //glEnableVertexAttribArray(2);
            //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

            // instanced
            // also set instance data
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, sphereMatrixVBO); // this attribute comes from a different vertex buffer
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, sphereColorVBO); // this attribute comes from a different vertex buffer
            glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glVertexAttribDivisor(1, 1); // tell OpenGL this is an instanced vertex attribute.
            glVertexAttribDivisor(2, 1);

            std::cout << "7" << glGetError() << std::endl;
        }

        /*SceneLights(unsigned int const & max_lights, int const& num_lights, float const& lights_height) :
            num_lights(num_lights),
            lights_height(lights_height),
            ubo_light_block(),
            max_lights(max_lights)
        {
            initialise();
        }*/

		void draw_debug_balls(glm::mat4 const & view_mat, glm::mat4 const& proj_mat)
        {
            sphereShader.use();

            sphereShader.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            sphereShader.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(view_mat));
            sphereShader.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(proj_mat));

            glBindVertexArray(sphereVAO);
            //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene->lights->sphereEBO);
            glDrawElementsInstanced(GL_TRIANGLES, sphere.get_meshes()[0].indices.size(), GL_UNSIGNED_INT, nullptr, get_num_lights());

            glBindVertexArray(0);
        }

        constexpr size_t size() const noexcept
        {
            return lights.size();
        }

        constexpr size_t size_bytes() const noexcept
        {
            return size() * sizeof(Light);
        }

        constexpr Light const* data() const noexcept
        {
            return lights.data();
        }

        uint32_t const & get_num_lights() const noexcept
        {
            return num_lights;
        }

    	void set_num_lights(uint32_t const & new_num_lights)
        {
            if (new_num_lights > max_lights)
            {
                throw std::runtime_error("num_lights > max_lights");
            }
            num_lights = new_num_lights;
        }

        constexpr size_t get_max_lights() const noexcept
        {
            return max_lights;
        }

        void set_light_heights(float const& new_light_heights)
        {
            lights_height = new_light_heights;
        }

        friend class UI;
    };
}