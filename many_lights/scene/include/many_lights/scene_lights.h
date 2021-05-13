#include <array>
#include <glm/glm.hpp>
#include "many_lights/colors.h"

template <size_t max_lights = 100>
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
    std::array<glm::vec4, max_lights> light_colors;

    void calculate_light_positions(int const& num_lights, float const& lights_height)
    {
        for (int light_index = 0; light_index < num_lights; ++light_index)
        {
            float x_pos = -1000.0f + ((light_index % 10) * (2000.0f / std::min(num_lights, 10)));
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
    SceneLights(int const& num_lights, float const& lights_height)
    {
        calculate_light_positions(num_lights, lights_height);
    }

    constexpr size_t size() const noexcept
    {
        return lights.size();
    }

    constexpr size_t size_bytes() const noexcept
    {
        return lights.size() * 2 * sizeof(glm::mat4);
    }

    constexpr Light const* data() const noexcept
    {
        return lights.data();
    }
};