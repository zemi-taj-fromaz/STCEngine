#pragma once

#include "VulkanInit.h"
#include "HelperObjects.h"
#include "Mesh.h"
#include "Layer.h"

#include <random>

class Renderable
{
public:
    Renderable(MeshWrapper* meshHandle, bool isSkybox) : MeshHandle(meshHandle), isSkybox(isSkybox)
    {}
    virtual ~Renderable() {};

    void compute_animation(float time);
    virtual void update(float time, const glm::vec3& cameraPosition) {}

    void bind_materials(VkCommandBuffer& commandBuffer, uint32_t imageIndex);
    void bind_mesh(VkCommandBuffer& commandBuffer);
    void bind_texture(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

    const glm::vec3& get_position() const { return Position; }
    const glm::vec3& get_direction() const { return Direction; }

    bool is_animated() const { return MeshHandle->animated != std::nullopt; }
    bool is_textured() const { return MeshHandle->texture != nullptr; }

    const std::shared_ptr<Pipeline>  get_pipeline() const { return MeshHandle->pipeline; }
    MeshWrapper* get_mesh() const { return this->MeshHandle; }
    const glm::mat4& get_model_matrix() const { return this->Model; }

    size_t get_indices_size() const { return this->MeshHandle->mesh.Indices.size(); }

public:
    void setTranslation(glm::mat4& translation);
    void setRotation(glm::mat4& rotation);
    void setScale(glm::mat4& scale);

protected:
    void compute_model_matrix();// { Model = Translation * Rotation * Scale; }

protected:
    MeshWrapper* MeshHandle;
    bool isSkybox;

    glm::mat4 Model{ glm::mat4(1.0f) };
    glm::vec3 Direction{ 0.0f, 0.0f, 1.0f };
    glm::vec3 Position{ 0.0f, 0.0f, 0.0f };
    glm::mat4 Translation{ glm::mat4(1.0f) };
    glm::mat4 Rotation{ glm::mat4(1.0f) };
    glm::mat4 Scale{ glm::mat4(1.0f) };

    glm::vec3 InitialRotation{ 0.0f, 0.0f, 1.0f };
};