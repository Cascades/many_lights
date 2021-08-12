#pragma once
#ifndef NDEBUG
#define GTEST_LANG_CXX11 1
#define _HAS_TR1_NAMESPACE 1
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "glm/gtc/type_ptr.hpp"
#include "many_lights/camera.h"
#include "many_lights/window.h"
#include "many_lights/renderer.h"
#include "many_lights/many_lights_algorithm.h"
#include "many_lights/many_lights.h"
#include "many_lights/benchmarker.h"
#include "many_lights/ui.h"

inline void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
}

inline void saveImage(const char* filepath, GLFWwindow* w) {
    int width, height;
    glfwGetFramebufferSize(w, &width, &height);
    GLsizei nrChannels = 3;
    GLsizei stride = nrChannels * width;
    stride += (stride % 4) ? (4 - stride % 4) : 0;
    GLsizei bufferSize = stride * height;
    std::vector<char> buffer(bufferSize);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    stbi_flip_vertically_on_write(true);
    stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
}

namespace ml
{
	template<size_t max_lights>
	class ManyLights
	{
	private:
		std::shared_ptr<ml::Camera> camera;
		ml::Window window;

	// REVERT THIS HACK
	public:
		std::shared_ptr<ml::SceneEntities> models;
	private:
		std::shared_ptr<ml::SceneLights<max_lights>> lights;
		std::shared_ptr<ml::Scene<max_lights>> scene;

        std::vector<std::shared_ptr<ml::ManyLightsAlgorithm<max_lights>>> algorithms;
        std::shared_ptr<ml::ManyLightsAlgorithm<max_lights>> current_algorithm;

		ml::Renderer renderer;

        ml::UI user_interface;

    public:
        ManyLights();

		void add_model(std::filesystem::path const& model) const;

		void set_lights(int const& num_lights, float const& lights_height) const;

		//template <class T, class... ArgTypes>
		//void set_algorithm(ArgTypes&&... args, ml::Scene<max_lights> const& scene);

        template <class T, class... ArgTypes>
        void add_algorithm(ArgTypes&&... args);

        ml::Scene<max_lights> const& get_scene();
		
		void run();
	};

    template <size_t max_lights>
    ml::Scene<max_lights> const& ManyLights<max_lights>::get_scene()
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
    void ManyLights<max_lights>::add_algorithm(ArgTypes&&... args)
    {
        algorithms.push_back(std::make_shared<T>(args...));
        algorithms.back()->init(window.get_width(), window.get_height(), *scene);
        current_algorithm = algorithms.back();
    }

	// primary constructor
    template<size_t max_lights>
    ManyLights<max_lights>::ManyLights() :
        camera(std::move(std::make_shared<ml::Camera>())),
        window(camera),
        models(std::move(std::make_shared<ml::SceneEntities>())),
        lights(std::move(std::make_shared<ml::SceneLights<max_lights>>())),
        scene(std::move(std::make_shared<ml::Scene<max_lights>>(camera, models, lights))),
        renderer(std::move(ml::Renderer())),
		user_interface()
    {
       // glEnable(GL_DEBUG_OUTPUT);
       // glDebugMessageCallback(MessageCallback, 0);
        window.setup_viewport(window.get_width(), window.get_height());
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
            user_interface.begin_ui(bm, *scene->lights, algorithms, scene);

        	// being benchmarking if required
            bm.begin_if_primed();
        	// render scene using user algorithm
            current_algorithm->render(*scene, window.get_window());

            /*scene->lights->sphereShader.use();

            scene->lights->sphereShader.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            scene->lights->sphereShader.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(scene->camera->GetViewMatrix()));
            scene->lights->sphereShader.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(scene->camera->projection_matrix));
        	
            glBindVertexArray(scene->lights->sphereVAO);
            //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene->lights->sphereEBO);
            glDrawElementsInstanced(GL_TRIANGLES, scene->lights->sphere.get_meshes()[0].indices.size(), GL_UNSIGNED_INT, nullptr, scene->lights->get_num_lights());
        	
            glBindVertexArray(0);*/
        	
        	// end benchmarking if begun
            bm.end_if_primed();

        	// finish render pass
            user_interface.end_ui();

        	// swap frames
            window.swap_buffers();

            if(user_interface.take_image)
            {
                char file_name[] = "snapshot.png";
            	
                saveImage(file_name, window.get_window());
                user_interface.take_image = false;
            }
        	
            glfwPollEvents();
        }

        bm.log_all_to_file();
        // end run()
    }
}
