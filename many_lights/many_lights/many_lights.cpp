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
#include "many_lights/scene_lights.h"
#include "many_lights/colors.h"

#include <memory>
#include <array>

int main()
{
    int old_num_lights = 70;
    float old_lights_height = 3.0f;

    int num_lights = 70;
    float lights_height = 3.0f;

    std::shared_ptr<ml::Camera> main_camera = std::make_shared<ml::Camera>();

    ml::Window window = ml::Window(main_camera);
    ml::intialise_glad();
    window.setup_viewport(800, 600);

    ml::Renderer renderer = ml::Renderer();

    ml::Shader basic_blinn_phong;
    ml::Shader camera_icon;

    try
    {
        basic_blinn_phong = std::move(ml::Shader("../assets/shaders/blinn_phong.vert", "../assets/shaders/blinn_phong.frag"));
        camera_icon = std::move(ml::Shader("../assets/shaders/camera_icon.vert", "../assets/shaders/camera_icon.frag"));
    }
    catch (ml::ShaderCompilationException const& e)
    {
        std::cout << e.what() << std::endl;
        throw e;
    }

    ml::Scene scene = { "../assets/sponza/sponza.obj" };

    for (auto& mesh : scene.get_sphere_model().get_meshes())
    {
        mesh.set_shader(camera_icon);
    }

    for (auto& model : scene.get_models())
    {
        for (auto& mesh : model.get_meshes())
        {
            mesh.set_shader(basic_blinn_phong);
        }
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);

    SceneLights lights(70, 3.0f);

    glUniformBlockBinding(*basic_blinn_phong.id, glGetUniformBlockIndex(*basic_blinn_phong.id, "Lights"), 0);

    glUniformBlockBinding(*camera_icon.id, glGetUniformBlockIndex(*camera_icon.id, "Lights"), 0);

    unsigned int uboLightBlock;
    glGenBuffers(1, &uboLightBlock);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLightBlock);
    glBufferData(GL_UNIFORM_BUFFER, lights.size_bytes(), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboLightBlock, 0, lights.size_bytes());

    glBindBuffer(GL_UNIFORM_BUFFER, uboLightBlock);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, lights.size_bytes(), lights.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    /*ml::Buffer<GL_UNIFORM_BUFFER> uboLightBlock = ml::Buffer<GL_UNIFORM_BUFFER>();
    uboLightBlock.bind();
    uboLightBlock.buffer_data(lights.size_bytes(), GL_DYNAMIC_DRAW);
    uboLightBlock.unbind();

    uboLightBlock.bind_buffer_range(static_cast<size_t>(0), static_cast<size_t>(0), lights.size_bytes());

    uboLightBlock.bind();
    uboLightBlock.buffer_sub_data(0, lights.size_bytes(), lights.data());
    uboLightBlock.unbind();*/

    int model_matrix_location = glGetUniformLocation(*basic_blinn_phong.id, "model");
    int view_matrix_location = glGetUniformLocation(*basic_blinn_phong.id, "view");
    int projection_matrix_location = glGetUniformLocation(*basic_blinn_phong.id, "projection");

    glm::vec3 object_color = glm::vec3(1.0f, 1.0f, 1.0f);

    int view_position_location = glGetUniformLocation(*basic_blinn_phong.id, "viewPos");
    int object_color_location = glGetUniformLocation(*basic_blinn_phong.id, "objectColor");

    int num_lights_location = glGetUniformLocation(*basic_blinn_phong.id, "num_lights");

    glEnable(GL_DEPTH_TEST);

    float delta_time = 0.0f;
    float last_time = 0.0f;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window.get_window(), true);
    ImGui_ImplOpenGL3_Init("#version 460");
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ml::BenchMarker bm{};

    while (!glfwWindowShouldClose(window.get_window()))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        window.process_input(delta_time);

        renderer.render();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Demo window");
        ImGui::SliderInt("num_lights", &num_lights, 0, 100);
        ImGui::SliderFloat("lights_height", &lights_height, 1.0f, 1200.0f);
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
        
        basic_blinn_phong.use();
        glm::vec3 view_position = main_camera->position;
        glm::mat4 view_matrix = main_camera->GetViewMatrix();
        glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), (float)window.get_width() / (float)window.get_height(), 1.0f, 10000.0f);

        glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glUniform1iv(num_lights_location, 1, &num_lights);

        glUniform3fv(view_position_location, 1, glm::value_ptr(view_position));
        glUniform3fv(object_color_location, 1, glm::value_ptr(object_color));

        if (bm)
        {
            bm.begin();
        }
        for (auto const& model : scene.get_models())
        {
            model.draw(basic_blinn_phong);
        }
        if (bm)
        {
            bm.end_and_log();
        }

        camera_icon.use();
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        for (int light_index = 0; light_index < num_lights; ++light_index)
        {
            glUniform1i(3, light_index);
            scene.get_sphere_model().draw(camera_icon);
        }

        /*if (num_lights != old_num_lights || lights_height != old_lights_height)
        {
            calculate_light_positions(num_lights, lights_height, light_model_matrices, lights);

            glBindBuffer(GL_UNIFORM_BUFFER, uboLightBlock);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, lights.size() * 2 * sizeof(glm::mat4), lights.data());
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            old_lights_height = lights_height;
            old_num_lights = num_lights;
        }*/

        // Render dear imgui into screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
