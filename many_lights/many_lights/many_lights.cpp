#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "many_lights/many_lights.h"
#include "many_lights/window.h"
#include "many_lights/glad.h"
#include "many_lights/renderer.h"
#include "many_lights/model.h"
#include "many_lights/camera.h"

#include <memory>

int main()
{
    std::shared_ptr<ml::Camera> main_camera = std::make_shared<ml::Camera>();

    ml::Window window = ml::Window(main_camera);
    ml::intialise_glad();
    window.setup_viewport(800, 600);

    ml::Renderer renderer = ml::Renderer();

    ml::Shader basic_blinn_phong("../assets/shaders/blinn_phong.vert", "../assets/shaders/blinn_phong.frag");
    ml::Model crytek_model("../assets/sponza/sponza.obj");

    glm::mat4 model_matrix = glm::mat4(1.0f);

    int model_matrix_location = glGetUniformLocation(basic_blinn_phong.ID, "model");
    int view_matrix_location = glGetUniformLocation(basic_blinn_phong.ID, "view");
    int projection_matrix_location = glGetUniformLocation(basic_blinn_phong.ID, "projection");

    //HARD CODED FOR NOW
    glm::vec3 light0_position = glm::vec3(-250.0f, 250.0f, 0.0f);
    glm::vec3 light0_color = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 light1_position = glm::vec3(250.0f, 250.0f, 0.0f);
    glm::vec3 light1_color = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 object_color = glm::vec3(1.0f, 1.0f, 1.0f);

    int light0_position_location = glGetUniformLocation(basic_blinn_phong.ID, "lightPos0");
    int light0_color_location = glGetUniformLocation(basic_blinn_phong.ID, "lightColor0");
    int light1_position_location = glGetUniformLocation(basic_blinn_phong.ID, "lightPos1");
    int light1_color_location = glGetUniformLocation(basic_blinn_phong.ID, "lightColor1");
    int view_position_location = glGetUniformLocation(basic_blinn_phong.ID, "viewPos");
    int object_color_location = glGetUniformLocation(basic_blinn_phong.ID, "objectColor");

    glEnable(GL_DEPTH_TEST);

    float delta_time = 0.0f;
    float last_time = 0.0f;

    while (!glfwWindowShouldClose(window.get_window()))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_time;
        last_time = current_time;

        window.process_input(delta_time);
        
        basic_blinn_phong.use();
        glm::vec3 view_position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::mat4 view_matrix = main_camera->GetViewMatrix();
        glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), (float)window.get_width() / (float)window.get_height(), 1.0f, 10000.0f);

        //model_matrix = glm::rotate(model_matrix, static_cast<float>(glfwGetTime()) * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
        //glfwSetTime(0.0);

        glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glUniform3fv(light0_position_location, 1, glm::value_ptr(light0_position));
        glUniform3fv(light0_color_location, 1, glm::value_ptr(light0_color));
        glUniform3fv(light1_position_location, 1, glm::value_ptr(light1_position));
        glUniform3fv(light1_color_location, 1, glm::value_ptr(light1_color));
        glUniform3fv(view_position_location, 1, glm::value_ptr(view_position));
        glUniform3fv(object_color_location, 1, glm::value_ptr(object_color));

        renderer.render();
        crytek_model.draw(basic_blinn_phong);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
