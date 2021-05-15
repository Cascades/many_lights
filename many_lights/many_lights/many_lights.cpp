#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "many_lights/many_lights.h"
#include "many_lights/window.h"
#include "many_lights/glad.h"
#include "many_lights/renderer.h"
#include "many_lights/model.h"
#include "many_lights/camera.h"
#include "many_lights/exceptions.h"
#include "many_lights/benchmarker.h"
#include "many_lights/buffer.h"
#include "many_lights/scene.h"
#include "many_lights/scene_entities.h"
#include "many_lights/scene_lights.h"
#include "many_lights/ui.h"
#include "many_lights/many_lights_algorithm.h"

#include <memory>
#include <array>

int main()
{
    std::shared_ptr<ml::Camera> camera = std::make_shared<ml::Camera>();

    ml::Window window = ml::Window(camera);
    ml::intialise_glad();
    window.setup_viewport(800, 600);

    std::shared_ptr<ml::SceneEntities> models = std::make_shared<ml::SceneEntities>(std::initializer_list<std::string>{"../assets/sponza/sponza.obj" });
    std::shared_ptr<ml::SceneLights> lights = std::make_shared<ml::SceneLights>(200, 70, 3.0f);

    std::shared_ptr<ml::Scene> scene = std::make_shared<ml::Scene>(camera, models, lights);

    ml::Renderer renderer = ml::Renderer();

    float delta_time = 0.0f;
    float last_time = 0.0f;

    ml::UI user_interface = ml::UI();
    user_interface.init(window);

    ml::BenchMarker bm{};

    //DeferredRenderData drd{};
    //drd.init(window.get_width(), window.get_height());
    ForwardRenderData frd{};
    frd.init(window.get_width(), window.get_height());

    int old_width = window.get_width();
    int old_height = window.get_height();

    while (!glfwWindowShouldClose(window.get_window()))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        window.process_input(delta_time);
        if (window.get_width() != old_width || window.get_height() != old_height)
        {
            //drd.adjust_size(window.get_width(), window.get_height());
            old_width = window.get_width();
            old_height = window.get_height();
        }

        renderer.render();

        user_interface.begin_ui(bm, *scene->lights);

        bm.begin_if_primed();
        renderer.forward_render(*scene, frd);
        //renderer.deferred_render(*scene, drd);
        bm.end_if_primed();

        user_interface.end_ui();

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
