#include "Camera.h"
#include "VulkanInit.h"
#include "Layer.h"

void Camera::process_mouse_movement(float xoffset, float yoffset)
{
    Yaw += xoffset;
    Pitch += yoffset;

    if (Pitch > 89.0f)
        Pitch = 89.0f;
    if (Pitch < -89.0f)
        Pitch = -89.0f;

    direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    direction.y = sin(glm::radians(Pitch));
    direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(direction);
    Right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), Front));
    Up = glm::cross(Front, Right);

    update_object(Position, this->Front);

}



void Camera::set_field_of_view(float yoffset)
{
    Fov -= yoffset;
    if (Fov < 5.f)
        Fov = 5.0f;
    if (Fov > 45.0f)
        Fov = 45.0f;
}

void Camera::update_position(glm::vec3 position)
{
    Position += position;
    update_object(Position, this->Front);
}

void Camera::update_object(glm::vec3 position, glm::vec3 Front)
{
    if (!mesh) return;
    mesh->update_position(position + 50.0f * Front, Front);
}

