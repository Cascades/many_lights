#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "many_lights/scene_lights.h"
#include "many_lights/window.h"

namespace ml
{
	class UI
	{
    public:
		UI() = default;
		~UI() = default;

        int current_index;

        bool num_lights_changed = false;
        bool light_heights_changed = false;

		void init(ml::Window& win)
		{
			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			// Setup Platform/Renderer bindings
			ImGui_ImplGlfw_InitForOpenGL(win.get_window(), true);
			ImGui_ImplOpenGL3_Init("#version 460");
			// Setup Dear ImGui style
			ImGui::StyleColorsDark();
		}

		template <size_t max_lights>
		void begin_ui(ml::BenchMarker& bm, ml::SceneLights<max_lights>& lights, std::vector<std::shared_ptr<ml::ManyLightsAlgorithm<max_lights>>> const & algorithms, std::shared_ptr<ml::Scene<max_lights>> scene)
		{
            num_lights_changed = false;
            light_heights_changed = false;
			
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Demo window");
            //int power_of_lights = lights.light_power;
            int int_num_lights = lights.get_num_lights();
			// have to use int for imgui
            if (ImGui::SliderInt("num_lights", &int_num_lights, 0, lights.get_max_lights()))
            {
                num_lights_changed = true;
            	
                lights.num_lights = int_num_lights;
                lights.calculate_lighting();

                lights.ubo_light_block.bind();
                lights.ubo_light_block.buffer_sub_data(0, lights.size_bytes(), lights.data());
                lights.ubo_light_block.unbind();

                glBindBuffer(GL_ARRAY_BUFFER, lights.sphereMatrixVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * max_lights, &lights.light_model_positions[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ARRAY_BUFFER, lights.sphereColorVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * max_lights, &lights.light_colors[0], GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                
            }

            if(ImGui::SliderFloat("brightness_factor", &lights.light_intensity_factor, 0.0, 1.0))
            {
                lights.calculate_lighting();
            }

           // ImGui::Text(std::to_string(lights.num_lights).c_str());
			
            if (ImGui::SliderFloat("lights_height", &lights.lights_height, 1.0f, 1200.0f))
            {
                light_heights_changed = true;
            	
                lights.calculate_lighting();

                lights.ubo_light_block.bind();
                lights.ubo_light_block.buffer_sub_data(0, lights.size_bytes(), lights.data());
                lights.ubo_light_block.unbind();
            }

            for (int i = 0; i < algorithms.size(); ++i)
            {
                if(ImGui::RadioButton(algorithms[i]->get_name().c_str(), &current_index, i))
                {
                    num_lights_changed = true;
                    light_heights_changed = true;
                }
                ImGui::Indent(16.0f);
                algorithms[i]->ui(num_lights_changed, light_heights_changed, scene);
                ImGui::Unindent(16.0f);
            }
            
            if (bm)
            {
                if (ImGui::Button("Finish Benchmark"))
                {
                    bm.diffuse();
                    bm.log_all_to_file();
                }
            }
            else
            {
                if (ImGui::Button("Start Benchmark"))
                {
                    bm.prime();
                }
            }
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Position: %.3f, %.3f, %.3f", scene->camera->position.x, scene->camera->position.y, scene->camera->position.z);
            ImGui::End();
		}

        void end_ui()
        {
            // Render dear imgui into screen
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
	};
}
