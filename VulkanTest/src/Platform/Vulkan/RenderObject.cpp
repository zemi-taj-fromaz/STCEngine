#pragma once

#include "RenderObject.h"


RenderObject::RenderObject(const RenderObject& renderObject)
{
    MeshHandle = renderObject.MeshHandle;
    MaterialHandle = renderObject.MaterialHandle;
    isSkybox = renderObject.isSkybox;
    Model = renderObject.Model;
    Translation = renderObject.Translation;
    Rotation = renderObject.Rotation;
    Scale = renderObject.Scale;
}


void RenderObject::setTranslation(glm::mat4& translation)
{
    Translation = translation;
    compute_model_matrix();
}
void RenderObject::setRotation(glm::mat4& rotation)
{
    Rotation = rotation;
    compute_model_matrix();
}
void RenderObject::setScale(glm::mat4& scale)
{
    Scale = scale;
    compute_model_matrix();
}


void RenderObject::compute_animation(float time)
{
    int segment = static_cast<int>(std::floor(time));

    time -= std::floor(time);

    auto animation = this->MeshHandle->Animation;

    glm::mat4x3 R = {
            {animation[segment % animation.size()]},
            {animation[(segment + 1) % animation.size()]},
            {animation[(segment + 2) % animation.size()]},
            {animation[(segment + 3) % animation.size()]}
    };
    glm::vec4 T = glm::vec4(glm::pow(time, 3.0f), glm::pow(time, 2.0f), time, 1.0f);

    setTranslation(glm::translate(glm::mat4(1.0f), glm::vec3(R * this->MeshHandle->B * T / 6.0f)));

    glm::vec3 T_d = glm::vec3(glm::pow(time, 2.0f), time, 1.0f);

    glm::vec3 GoalRotation = glm::vec3(glm::mat3(R * this->MeshHandle->B_d) * T_d) / 2.0f;
    glm::vec3 InitialRotation = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 rotationAxis = glm::normalize(glm::cross(InitialRotation, GoalRotation));
    float rotationAngle = std::acos(glm::dot(glm::normalize(InitialRotation), glm::normalize(GoalRotation))) * 180.0f / static_cast<float>(M_PI);

    //setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis));
}

void RenderObject::bind_materials(VkCommandBuffer& commandBuffer, std::vector<VkDescriptorSet*> descriptorSets, uint32_t imageIndex)
{
    for (int i = 0; i < descriptorSets.size(); ++i)
    {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->MaterialHandle->PipelineLayout, i, 1, descriptorSets[i], 0, nullptr);
    }

    if (this->MaterialHandle->TextureSets[0] != VK_NULL_HANDLE) vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->MaterialHandle->PipelineLayout, descriptorSets.size(), 1, &MaterialHandle->TextureSets[imageIndex], 0, nullptr);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->MaterialHandle->Pipeline);
}

void RenderObject::bind_mesh(VkCommandBuffer& commandBuffer)
{
    VkBuffer vertexBuffers[] = { this->MeshHandle->VertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, this->MeshHandle->IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
}
