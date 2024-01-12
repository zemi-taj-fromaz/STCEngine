#include "LightProperties.h"
#include "Renderable.h"

LightProperties::LightProperties(LightType type, glm::vec3 diffColor, glm::vec3 direction) : LightProperties(type, diffColor)
{
    this->direction = direction;
    this->update_light = [](float time, glm::vec3 camera_position, Renderable* renderable)
    {
        static const float rotationSpeed = static_cast<float>(glm::two_pi<float>()) / 10.0f; // Radians per second for a full rotation in 10 seconds

        float angle = rotationSpeed * time;
        auto Model = renderable->get_model();
        auto Position = renderable->get_position();
        auto Direction = renderable->get_direction();
        Model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * Model;
        Position = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(Position, 1.0f);
        Direction = -Position;
    };
}
