#pragma once

#include "VulkanInit.h"

struct MeshWrapper;
struct LightProperties;

struct Camera
{
    Camera()
    {
        // std::cout << direction.x << " " << direction.y << direction.z << std::endl;
    }

    glm::vec3 Position{ glm::vec3(0.0f, 00.0f, 100.0f) };

    float Yaw = -90.0f;
    float Pitch = 0.0f;
    float Fov = 45.0f;

    glm::vec3 direction{
        cos(glm::radians(Yaw)) * cos(glm::radians(Pitch)),
        sin(glm::radians(Pitch)),
        sin(glm::radians(Yaw)) * cos(glm::radians(Pitch))
    };


    glm::vec3 Front{ glm::normalize(direction) };
    glm::vec3 Right{ glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), Front)) };
    glm::vec3 Up{ glm::cross(Front, Right) };


    void process_mouse_movement(float xoffset, float yoffset);
    void set_field_of_view(float yoffset);
    void update_position(glm::vec3 position);

    void update_object(glm::vec3 position, glm::vec3 Front);

     std::shared_ptr<MeshWrapper> mesh;
     std::shared_ptr<LightProperties> cameraLight;
};
