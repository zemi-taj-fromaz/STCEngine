#include "RenderLight.h"

void RenderLight::update(float time, const glm::vec3& cameraPosition)
{
    //TODO - neka lightProperties ima update function pa se svako svjetlo zasbeno update-a // meshWrapper->lightProperties->Update() i ovo je defaultna implementacija
    
    this->LightProperties->update_light(time, cameraPosition, this);
}
