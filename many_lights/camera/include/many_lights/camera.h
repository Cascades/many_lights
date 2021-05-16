#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace ml
{
    enum class CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    class Camera
    {
    public:
        glm::mat4 projection_matrix;
        glm::vec3 position;
        glm::vec3 world_up;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;

        float yaw;
        float pitch;

        float movenent_speed;
        float mouse_sensitivity;
        float zoom;

        Camera();
    	
        Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, int const& width, int const& height);

        void set_projection_matrix(float const& fov, float const& width, float const& height, float const& near_z, float const& far_z);

        glm::mat4 GetViewMatrix();

        void ProcessKeyboard(ml::CameraMovement direction, float deltaTime);

        void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

        void ProcessMouseScroll(float yoffset);

    private:
        void updateCameraVectors();
    };
}