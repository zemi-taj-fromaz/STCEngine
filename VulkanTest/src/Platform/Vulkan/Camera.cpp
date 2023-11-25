#include "Camera.h"
#include "VulkanInit.h"

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
}

void Camera::set_field_of_view(float yoffset)
{
    Fov -= yoffset;
    if (Fov < 1.0f)
        Fov = 1.0f;
    if (Fov > 45.0f)
        Fov = 45.0f;
}

