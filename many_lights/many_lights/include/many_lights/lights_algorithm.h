#pragma once

class LightsAlgorithm
{
protected:
	struct Light
	{
		glm::vec4 position;
		glm::vec4 color;
	};

	std::array<Light, 100> lights;
	std::array<glm::mat4, 100> light_model_matrices;
	std::array<glm::vec4, 100> light_colors;
public:
	LightsAlgorithm();
	virtual ~LightsAlgorithm();

	virtual draw() = 0;
};