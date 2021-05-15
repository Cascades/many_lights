#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "many_lights/renderer.h"
#include "many_lights/scene.h"
#include "many_lights/scene_lights.h"
#include "many_lights/camera.h"
#include "many_lights/exceptions.h"

#include <iostream>

void ml::Renderer::render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ml::Renderer::forward_render(ml::Scene& scene, ForwardRenderData& frd)
{
	glEnable(GL_DEPTH_TEST);

	glm::vec3 view_position = scene.camera->position;
	glm::mat4 view_matrix = scene.camera->GetViewMatrix();
	glm::mat4 projection_matrix = scene.camera->projection_matrix;

	glUniformMatrix4fv(frd.model_matrix_location, 1, GL_FALSE, glm::value_ptr(frd.model_matrix));
	glUniformMatrix4fv(frd.view_matrix_location, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(frd.projection_matrix_location, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	glUniform1uiv(frd.num_lights_location, 1, &scene.lights->get_num_lights());

	glUniform3fv(frd.view_position_location, 1, glm::value_ptr(view_position));
	glUniform3fv(frd.object_color_location, 1, glm::value_ptr(frd.object_color));

	for (auto& model : scene.models->get_models())
	{
		model.draw(frd.forward_bling_phong);
	}
}

void ml::Renderer::deferred_render(ml::Scene& scene, DeferredRenderData& drd)
{
	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, drd.g_buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drd.geometry_pass_shader.use();

	glUniformMatrix4fv(glGetUniformLocation(drd.geometry_pass_shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
	glUniformMatrix4fv(glGetUniformLocation(drd.geometry_pass_shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(scene.camera->GetViewMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(drd.geometry_pass_shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(scene.camera->projection_matrix));

	for (auto& model : scene.models->get_models())
	{
		model.draw(drd.geometry_pass_shader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/////////////////////////////////////////////////

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drd.light_pass_shader.use();

	glUniform3fv(glGetUniformLocation(drd.light_pass_shader.ID, "viewPos"), 1, glm::value_ptr(scene.camera->position));
	glUniform1uiv(glGetUniformLocation(drd.light_pass_shader.ID, "num_lights"), 1, &scene.lights->get_num_lights());

	glUniform1i(glGetUniformLocation(drd.light_pass_shader.ID, "g_position"), 0);
	glUniform1i(glGetUniformLocation(drd.light_pass_shader.ID, "g_normal"), 1);
	glUniform1i(glGetUniformLocation(drd.light_pass_shader.ID, "g_diff_spec"), 2);
	glUniform1i(glGetUniformLocation(drd.light_pass_shader.ID, "g_ambient"), 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, drd.g_position);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, drd.g_normal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, drd.g_diff_spec);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, drd.g_ambient);

	glBindVertexArray(drd.quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}