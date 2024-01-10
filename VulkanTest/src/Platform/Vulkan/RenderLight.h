#pragma once

#include "Renderable.h"

class RenderLight : public Renderable
{
public:
    RenderLight(std::shared_ptr<LightProperties> lightProperties) : LightProperties(lightProperties) {}

    RenderLight(MeshWrapper* meshHandle) : Renderable(meshHandle)
    {
        this->LightProperties = this->MeshHandle->lightProperties;
        // this->MeshHandle->object = std::shared_ptr<RenderObject>(this);
    }

    RenderLight(const RenderLight& renderObject);

    void update(float time, const glm::vec3& cameraPosition) override;

    std::shared_ptr<LightProperties>& get_light_properties() { return LightProperties; }
    bool is_light_source() const override { return this->LightProperties != nullptr; }

    ~RenderLight() {}

private:
    std::shared_ptr<LightProperties> LightProperties;

    glm::vec4 position;
    glm::vec4 ambientColor;
    glm::vec4 diffColor;
    glm::vec4 specColor;
    glm::vec4 clq;

    glm::vec4 direction;
    float innerCutoff;
    float outerCutoff;

};
