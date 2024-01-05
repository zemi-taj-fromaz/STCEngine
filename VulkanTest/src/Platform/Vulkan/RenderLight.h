#pragma once

#include "Renderable.h"

class RenderLight : public Renderable
{
public:
    // RenderObject(){}

    RenderLight(MeshWrapper* meshHandle) : Renderable(meshHandle)
    {

        // this->MeshHandle->object = std::shared_ptr<RenderObject>(this);
    }

    RenderLight(const RenderLight& renderObject);

    void update(float time, const glm::vec3& cameraPosition) override;



    ~RenderLight() {}

private:
    glm::vec4 position;
    glm::vec4 ambientColor;
    glm::vec4 diffColor;
    glm::vec4 specColor;
    glm::vec4 clq;

    glm::vec4 direction;
    float innerCutoff;
    float outerCutoff;

};
