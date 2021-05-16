#include "test_application/forward_renderer.h"
#include <many_lights/exceptions.h>

#include "glm/gtc/type_ptr.hpp"


TestApplication::ForwardBlinnPhong::ForwardBlinnPhong()
{
	ForwardBlinnPhong::init(800, 600);
}

TestApplication::ForwardBlinnPhong::ForwardBlinnPhong(int const& width, int const& height)
{
	ForwardBlinnPhong::init(width, height);
}

void TestApplication::ForwardBlinnPhong::init(int const& width, int const& height)
{
	try
	{
		forward_blinn_phong = ml::Shader("../assets/shaders/forward_blinn_phong.vert", "../assets/shaders/forward_blinn_phong.frag");
	}
	catch (ml::ShaderCompilationException const& e)
	{
		std::cout << e.what() << std::endl;
		throw e;
	}

	glUniformBlockBinding(*forward_blinn_phong.id, glGetUniformBlockIndex(*forward_blinn_phong.id, "Lights"), 0);
}

void TestApplication::ForwardBlinnPhong::adjust_size(int const& width, int const& height)
{
	return;
}

void TestApplication::ForwardBlinnPhong::render(ml::Scene& scene)
{
	glEnable(GL_DEPTH_TEST);

	glm::vec3 view_position = scene.camera->position;
	glm::mat4 view_matrix = scene.camera->GetViewMatrix();
	glm::mat4 projection_matrix = scene.camera->projection_matrix;

	forward_blinn_phong.use();

	forward_blinn_phong.set_mat_4x4_floatv("model", 1, GL_FALSE, glm::value_ptr(model_matrix));
	forward_blinn_phong.set_mat_4x4_floatv("view", 1, GL_FALSE, glm::value_ptr(view_matrix));
	forward_blinn_phong.set_mat_4x4_floatv("projection", 1, GL_FALSE, glm::value_ptr(projection_matrix));

	forward_blinn_phong.set_uintv("num_lights", 1, &scene.lights->get_num_lights());

	forward_blinn_phong.set_floatv3("viewPos", 1, glm::value_ptr(view_position));

	for (auto& model : scene.models->get_models())
	{
		model.draw(forward_blinn_phong);
	}
}