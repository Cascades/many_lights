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
		void begin_ui(ml::BenchMarker& bm, ml::SceneLights<max_lights>& lights, std::vector<std::shared_ptr<ml::ManyLightsAlgorithm>> const & algorithms)
		{
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Demo window");
            int int_num_lights = lights.get_num_lights();
			// have to use int for imgui
            if (ImGui::SliderInt("num_lights", &int_num_lights, 0, static_cast<int>(lights.get_max_lights())))
            {
                lights.num_lights = int_num_lights;
                lights.calculate_light_positions();

                lights.ubo_light_block.bind();
                lights.ubo_light_block.buffer_sub_data(0, lights.size_bytes(), lights.data());
                lights.ubo_light_block.unbind();


            }
            if (ImGui::SliderFloat("lights_height", &lights.lights_height, 1.0f, 1200.0f))
            {
                lights.calculate_light_positions();

                lights.ubo_light_block.bind();
                lights.ubo_light_block.buffer_sub_data(0, lights.size_bytes(), lights.data());
                lights.ubo_light_block.unbind();
            }

            for (int i = 0; i < algorithms.size(); ++i)
            {
                ImGui::RadioButton(algorithms[i]->get_name().c_str(), &current_index, i);
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
