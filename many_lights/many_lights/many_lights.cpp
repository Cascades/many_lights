#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "many_lights/many_lights.h"
#include "many_lights/window.h"
#include "many_lights/glad.h"
#include "many_lights/renderer.h"
#include "many_lights/model.h"

int main()
{
    ml::Window window = ml::Window();
    ml::intialise_glad();
    window.setup_viewport(800, 600);

    ml::Renderer renderer = ml::Renderer();

    ml::Shader basic_blinn_phong("../assets/shaders/blinn_phong.vert", "../assets/shaders/blinn_phong.frag");
    ml::Model crytek_model("../assets/cube/cube.obj");

    glm::mat4 model_matrix = glm::mat4(1.0f);

    int model_matrix_location = glGetUniformLocation(basic_blinn_phong.ID, "model");
    int view_matrix_location = glGetUniformLocation(basic_blinn_phong.ID, "view");
    int projection_matrix_location = glGetUniformLocation(basic_blinn_phong.ID, "projection");

    //uniform vec3 lightPos;
    //uniform vec3 viewPos;
    //uniform vec3 lightColor;
    //uniform vec3 objectColor;

    glm::vec3 light_position = glm::vec3(100.0f, 0.0f, 100.0f);
    glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 object_color = glm::vec3(1.0f, 0.1f, 0.1f);

    int light_position_location = glGetUniformLocation(basic_blinn_phong.ID, "lightPos");
    int view_position_location = glGetUniformLocation(basic_blinn_phong.ID, "viewPos");
    int light_color_location = glGetUniformLocation(basic_blinn_phong.ID, "lightColor");
    int object_color_location = glGetUniformLocation(basic_blinn_phong.ID, "objectColor");

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window.get_window()))
    {
        window.process_input();
        
        basic_blinn_phong.use();
        glm::vec3 view_position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::mat4 view_matrix = glm::lookAt(view_position,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), (float)800 / (float)600, 0.1f, 100.0f);

        model_matrix = glm::rotate(model_matrix, static_cast<float>(glfwGetTime()) * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
        glfwSetTime(0.0);

        glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        glUniform3fv(light_position_location, 1, glm::value_ptr(light_position));
        glUniform3fv(view_position_location, 1, glm::value_ptr(view_position));
        glUniform3fv(light_color_location, 1, glm::value_ptr(light_color));
        glUniform3fv(object_color_location, 1, glm::value_ptr(object_color));

        renderer.render();
        crytek_model.draw(basic_blinn_phong);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
