#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "many_lights/many_lights.h"
#include "many_lights/benchmarker.h"
#include "many_lights/ui.h"

#include <memory>

ml::ManyLights::ManyLights() :
    camera(std::move(std::make_shared<ml::Camera>())),
    window(camera),
    models(std::move(std::make_shared<ml::SceneEntities>())),
    lights(std::move(std::make_shared<ml::SceneLights>())),
    scene(std::move(std::make_shared<ml::Scene>(camera, models, lights))),
    renderer(std::move(ml::Renderer()))
{
    window.setup_viewport(800, 600);
}

void ml::ManyLights::add_model(std::filesystem::path const & model) const
{
    models->add_model(model);
}

void ml::ManyLights::set_lights(unsigned const& max_lights, int const& num_lights, float const& lights_height) const
{
    lights->set_max_lights(max_lights);
    lights->set_num_lights(num_lights);
    lights->set_light_heights(lights_height);
    lights->initialise();
}

void ml::ManyLights::run()
{
    float delta_time = 0.0f;
    float last_time = 0.0f;

    ml::UI user_interface = ml::UI();
    user_interface.init(window);

    ml::BenchMarker bm{};

    int old_width = window.get_width();
    int old_height = window.get_height();

    // run()
    while (!glfwWindowShouldClose(window.get_window()))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        window.process_input(delta_time);
        if (window.get_width() != old_width || window.get_height() != old_height)
        {
            algorithm->adjust_size(window.get_width(), window.get_height());
            old_width = window.get_width();
            old_height = window.get_height();
        }

        renderer.render();

        user_interface.begin_ui(bm, *scene->lights);

        bm.begin_if_primed();
        algorithm->render(*scene);
        bm.end_if_primed();

        user_interface.end_ui();

        window.swap_buffers();
        glfwPollEvents();
    }
    // end run()
}
