#include "RenderLight.h"

void RenderLight::update(float time, const glm::vec3& cameraPosition)
{
    //TODO - neka lightProperties ima update function pa se svako svjetlo zasbeno update-a // meshWrapper->lightProperties->Update() i ovo je defaultna implementacija
    
    static const float rotationSpeed = static_cast<float>(glm::two_pi<float>()) / 10.0f; // Radians per second for a full rotation in 10 seconds

    float angle = rotationSpeed * time;
    this->Model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * this->Model;
    this->Position = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(Position, 1.0f);
    this->Direction = -this->Position;
}
