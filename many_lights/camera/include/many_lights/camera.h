#pragma once
#include <glm/gtc/matrix_transform.hpp>

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
        inline static const float default_yaw = -90.0f;
        inline static const float defualt_pitch = 0.0f;
        inline static const float default_speed = 200.0f;
        inline static const float default_sensitivity = 0.1f;
        inline static const float default_zoom = 45.0f;

        glm::mat4 projection_matrix;
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 world_up;

        float yaw;
        float pitch;

        float movenent_speed;
        float mouse_sensitivity;
        float zoom;

        Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = default_yaw, float pitch = defualt_pitch, int const& width = 800, int const& height = 600);

        Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, int const& width, int const& height);

        void set_projection_matrix(float const& fov, float const& width, float const& height, float const& near_z, float const& far_z);

        glm::mat4 GetViewMatrix();

        void ProcessKeyboard(ml::CameraMovement direction, float deltaTime);

        void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

        void ProcessMouseScroll(float yoffset);

    private:
        void updateCameraVectors();
    };
}