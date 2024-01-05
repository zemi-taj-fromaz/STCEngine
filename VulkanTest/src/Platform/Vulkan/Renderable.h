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
    {
        if (this->MeshHandle->scale.has_value()) 
        { 
            setScale(MeshHandle->scale.value()); 
        }
        if(this->MeshHandle->rotation.has_value()) 
        { 
            setRotation(MeshHandle->rotation.value());
        }
        if(this->MeshHandle->translation.has_value()) 
        { 
            setTranslation(MeshHandle->translation.value());
        }
      
    }
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
    MeshWrapper* get_mesh() { return this->MeshHandle; }
    const glm::mat4& get_model_matrix() const { return this->Model; }
    const glm::vec4& get_color() const { return this->MeshHandle->color; }

    size_t get_indices_size() const { return this->MeshHandle->mesh.Indices.size(); }

    bool is_billboard() const { return this->MeshHandle->Billboard; }
    void update_billboard(glm::vec3 CameraPosition);

    bool is_light_source() const { return this->MeshHandle->lightType != LightType::None;  }
    void update_light_source(float deltaTime);

public:
    void setTranslation(glm::mat4& translation);
    void setRotation(glm::mat4& rotation);
    void setScale(glm::mat4& scale);

    void update_position(glm::vec3 position, glm::vec3 Front);

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