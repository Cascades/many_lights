#pragma once
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "many_lights/colors.h"
#include "many_lights/buffer.h"

namespace ml
{
    class SceneLights
    {
    private:
        struct Light
        {
            glm::vec4 position;
            glm::vec4 color;
        };

        size_t max_lights = 100;
        std::vector<Light> lights;
        std::vector<glm::mat4> light_model_matrices;
        std::vector<glm::vec4> light_colors;
    	
        uint32_t num_lights = 70;
        float lights_height = 3.0f;

        ml::Buffer<GL_UNIFORM_BUFFER> ubo_light_block;

        void calculate_light_positions()
        {
            for (uint32_t light_index = 0; light_index < num_lights; ++light_index)
            {
                float x_pos = -1000.0f + ((light_index % 10) * (2000.0f / std::min(num_lights, 10u)));
                float y_pos = lights_height;
                float max_rows = (num_lights / 10) - 1;
                float curr_row = (light_index / 10);
                float z_pos = -((70 * max_rows) / 2) + (curr_row * 70) - 37;
                lights[light_index].position = glm::vec4(x_pos, y_pos, z_pos, 1.0);
                lights[light_index].color = glm::vec4(ml::utils::hsv_to_rgb(glm::vec3(light_index * (1.0f / num_lights), 1.0, 1.0)), 1.0);
                light_colors[light_index] = glm::vec4(ml::utils::hsv_to_rgb(glm::vec3(light_index * (1.0f / num_lights), 1.0, 1.0)), 1.0);
                light_model_matrices[light_index] = glm::mat4(1.0f);
                light_model_matrices[light_index] = glm::translate(light_model_matrices[light_index], glm::vec3(x_pos, y_pos, z_pos));
            }
        }

    public:
        SceneLights() = default;
    	
        void initialise()
        {
            lights.assign(max_lights, Light{});
            light_colors.assign(max_lights, glm::vec4(0.0f));
            light_model_matrices.assign(max_lights, glm::mat4(0.0));

            calculate_light_positions();

            ubo_light_block.buffer_data(size_bytes(), GL_DYNAMIC_DRAW);
            ubo_light_block.bind_buffer_range(static_cast<size_t>(0), static_cast<size_t>(0), size_bytes());
            ubo_light_block.buffer_sub_data(0, size_bytes(), data());
        }

        /*SceneLights(unsigned int const & max_lights, int const& num_lights, float const& lights_height) :
            num_lights(num_lights),
            lights_height(lights_height),
            ubo_light_block(),
            max_lights(max_lights)
        {
            initialise();
        }*/

        size_t size() const noexcept
        {
            return lights.size();
        }

        size_t size_bytes() const noexcept
        {
            return lights.size() * sizeof(Light);
        }

        Light const* data() const noexcept
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

        void set_max_lights(uint32_t const& new_max_lights)
        {
            max_lights = new_max_lights;
        }

        void set_light_heights(float const& new_light_heights)
        {
            lights_height = new_light_heights;
        }

        friend class UI;
    };
}