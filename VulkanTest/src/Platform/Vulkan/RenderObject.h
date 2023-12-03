#pragma once

#include "VulkanInit.h"
#include "HelperObjects.h"
#include "Mesh.h"


struct RenderObject
{
public:
    RenderObject() {}

    RenderObject(Mesh* meshHandle, Material* materialHandle, bool isSkybox) : MeshHandle(meshHandle), MaterialHandle(materialHandle), isSkybox(isSkybox)
    {}

    RenderObject(Mesh* meshHandle, Material* materialHandle) : RenderObject(meshHandle, materialHandle, false)
    {}

    RenderObject(const RenderObject& renderObject);



    void setTranslation(glm::mat4& translation);
    void setRotation(glm::mat4& rotation);
    void setScale(glm::mat4& scale);


    void compute_animation(float time);

    void bind_materials(VkCommandBuffer& commandBuffer, std::vector<VkDescriptorSet*> descriptorSets, uint32_t imageIndex);
    void bind_mesh(VkCommandBuffer& commandBuffer);

    Mesh* MeshHandle;
    Material* MaterialHandle;
    bool isSkybox;
    glm::mat4 Model{ glm::mat4(1.0f) };

private:

    glm::mat4 Translation{ glm::mat4(1.0f) };
    glm::mat4 Rotation{ glm::mat4(1.0f) };
    glm::mat4 Scale{ glm::mat4(1.0f) };

    void compute_model_matrix() { Model = Translation * Rotation * Scale; }
};