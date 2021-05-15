#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "many_lights/camera.h"

ml::Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, int const & width, int const & height) :
    front(glm::vec3(0.0f, 0.0f, -1.0f)),
    movenent_speed(default_speed),
    mouse_sensitivity(default_sensitivity),
    zoom(default_zoom)
{
    this->position = position;
    this->world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;
    this->set_projection_matrix(45.0f, width, height, 1.0f, 10000.0f);
    updateCameraVectors();
}

ml::Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, int const& width, int const& height) :
    front(glm::vec3(0.0f, 0.0f, -1.0f)),
    movenent_speed(default_speed),
    mouse_sensitivity(default_sensitivity),
    zoom(default_zoom)
{
    this->position = glm::vec3(posX, posY, posZ);
    this->world_up = glm::vec3(upX, upY, upZ);
    this->yaw = yaw;
    this->pitch = pitch;
    this->set_projection_matrix(45.0f, width, height, 1.0f, 10000.0f);
    updateCameraVectors();
}

void ml::Camera::set_projection_matrix(float const & fov, float const & width, float const & height, float const & near_z, float const & far_z)
{
    projection_matrix = glm::perspective(glm::radians(fov), width / height, near_z, far_z);
}

glm::mat4 ml::Camera::GetViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

void ml::Camera::ProcessKeyboard(ml::CameraMovement direction, float deltaTime)
{
    float velocity = movenent_speed * deltaTime;
    if (direction == ml::CameraMovement::FORWARD)
        position += front * velocity;
    if (direction == ml::CameraMovement::BACKWARD)
        position -= front * velocity;
    if (direction == ml::CameraMovement::LEFT)
        position -= right * velocity;
    if (direction == ml::CameraMovement::RIGHT)
        position += right * velocity;
}

void ml::Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void ml::Camera::ProcessMouseScroll(float yoffset)
{
    zoom -= (float)yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void ml::Camera::updateCameraVectors()
{
    glm::vec3 new_front;
    new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    new_front.y = sin(glm::radians(pitch));
    new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(new_front);
    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
}
