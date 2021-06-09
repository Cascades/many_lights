#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#include "many_lights/camera.h"
#include "many_lights/window.h"
#include "many_lights/renderer.h"
#include "many_lights/many_lights_algorithm.h"
#include "many_lights/many_lights.h"
#include "many_lights/benchmarker.h"
#include "many_lights/ui.h"


namespace ml
{
	template<size_t max_lights>
	class ManyLights
	{
	private:
		std::shared_ptr<ml::Camera> camera;
		ml::Window window;

		std::shared_ptr<ml::SceneEntities> models;
		std::shared_ptr<ml::SceneLights<max_lights>> lights;
		std::shared_ptr<ml::Scene> scene;

		std::unique_ptr<ml::ManyLightsAlgorithm> algorithm;

		ml::Renderer renderer;

    public:
		ManyLights();

		void add_model(std::filesystem::path const& model) const;

		void set_lights(int const& num_lights, float const& lights_height) const;

		template <class T, class... ArgTypes>
		void set_algorithm(ArgTypes&&... args);

		void run();
	};

	// set the algorithm to be tested
	template<size_t max_lights>
	template<class T, class ... ArgTypes>
	void ManyLights<max_lights>::set_algorithm(ArgTypes&&... args)
	{
		algorithm = std::make_unique<T>(args...);
		algorithm->init(window.get_width(), window.get_height());
	}

	// primary constructor
    template<size_t max_lights>
    ManyLights<max_lights>::ManyLights() :
        camera(std::move(std::make_shared<ml::Camera>())),
        window(camera),
        models(std::move(std::make_shared<ml::SceneEntities>())),
        lights(std::move(std::make_shared<ml::SceneLights<max_lights>>())),
        scene(std::move(std::make_shared<ml::Scene>(camera, models, lights))),
        renderer(std::move(ml::Renderer()))
    {
        window.setup_viewport(800, 600);
    }

	// add a model to the the scene
    template<size_t max_lights>
    void ManyLights<max_lights>::add_model(std::filesystem::path const& model) const
    {
        models->add_model(model);
    }

	// set light configuration
    template<size_t max_lights>
    void ManyLights<max_lights>::set_lights(int const& num_lights, float const& lights_height) const
    {
        lights->set_num_lights(num_lights);
        lights->set_light_heights(lights_height);
        lights->initialise();
    }

	// run the stored algorithm with benchmarking and UI
    template<size_t max_lights>
    void ManyLights<max_lights>::run()
    {
        float delta_time = 0.0f;
        float last_time = 0.0f;

        ml::UI user_interface = ml::UI();
        user_interface.init(window);

        ml::BenchMarker bm{};

        int old_width = window.get_width();
        int old_height = window.get_height();

        while (!glfwWindowShouldClose(window.get_window()))
        {
        	// timestep for camera inputs
            float current_time = static_cast<float>(glfwGetTime());
            delta_time = current_time - last_time;
            last_time = current_time;

        	// process indputs (incl. screen resize)
            window.process_input(delta_time);
            if (window.get_width() != old_width || window.get_height() != old_height)
            {
                algorithm->adjust_size(window.get_width(), window.get_height());
                old_width = window.get_width();
                old_height = window.get_height();
            }

        	// start rendering
            renderer.render();

        	// start ui render
            user_interface.begin_ui(bm, *scene->lights);

        	// being benchmarking if required
            bm.begin_if_primed();
        	// render scene using user algorithm
            algorithm->render(*scene);
        	// end benchmarking if begun
            bm.end_if_primed();

        	// finish render pass
            user_interface.end_ui();

        	// swap frames
            window.swap_buffers();
            glfwPollEvents();
        }

        bm.log_all_to_file();
        // end run()
    }
}
