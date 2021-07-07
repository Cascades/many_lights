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

        std::vector<std::shared_ptr<ml::ManyLightsAlgorithm>> algorithms;
        std::shared_ptr<ml::ManyLightsAlgorithm> current_algorithm;

		ml::Renderer renderer;

        ml::UI user_interface;

    public:
		ManyLights();

		void add_model(std::filesystem::path const& model) const;

		void set_lights(int const& num_lights, float const& lights_height) const;

		template <class T, class... ArgTypes>
		void set_algorithm(ArgTypes&&... args, ml::Scene const& scene);

        template <class T, class... ArgTypes>
        void add_algorithm(ArgTypes&&... args, ml::Scene const& scene);

        ml::Scene const& get_scene();
		
		void run();
	};

    template <size_t max_lights>
    ml::Scene const& ManyLights<max_lights>::get_scene()
    {
        return *scene;
    }


	// set the algorithm to be tested
	/*template<size_t max_lights>
	template<class T, class ... ArgTypes>
	void ManyLights<max_lights>::set_algorithm(ArgTypes&&... args, ml::Scene const & scene)
	{
    	
        current_algorithm = std::make_shared<T>(args...);
        current_algorithm->init(window.get_width(), window.get_height(), scene);
	}*/

    // set the algorithm to be tested
    template<size_t max_lights>
    template<class T, class ... ArgTypes>
    void ManyLights<max_lights>::add_algorithm(ArgTypes&&... args, ml::Scene const& scene)
    {
        algorithms.push_back(std::make_shared<T>(args...));
        algorithms.back()->init(window.get_width(), window.get_height(), scene);
        current_algorithm = algorithms.back();
    }

	// primary constructor
    template<size_t max_lights>
    ManyLights<max_lights>::ManyLights() :
        camera(std::move(std::make_shared<ml::Camera>())),
        window(camera),
        models(std::move(std::make_shared<ml::SceneEntities>())),
        lights(std::move(std::make_shared<ml::SceneLights<max_lights>>())),
        scene(std::move(std::make_shared<ml::Scene>(camera, models, lights))),
        renderer(std::move(ml::Renderer())),
		user_interface()
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
    	
        user_interface.init(window);

        size_t old_current_index = algorithms.size() - 1;
        user_interface.current_index = algorithms.size() - 1;

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
                current_algorithm->adjust_size(window.get_width(), window.get_height());
                old_width = window.get_width();
                old_height = window.get_height();
            }

        	if (old_current_index != user_interface.current_index)
        	{
                old_current_index = user_interface.current_index;

                current_algorithm = algorithms[old_current_index];
        	}

        	// start rendering
            renderer.render();

        	// start ui render
            user_interface.begin_ui(bm, *scene->lights, algorithms);

        	// being benchmarking if required
            bm.begin_if_primed();
        	// render scene using user algorithm
            current_algorithm->render(*scene);
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
