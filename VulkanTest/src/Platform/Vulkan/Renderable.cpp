#include "AppVulkanImpl.h"
#include "Renderable.h"
#include "RenderObject.h"

void Renderable::compute_model_matrix()
{
    this->Model = this->Translation * this->Rotation * this->Scale;
    this->Position = glm::vec3(this->Translation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Renderable::update_billboard(const glm::vec3& CameraPosition)
{

    glm::vec3 GoalRotation = glm::normalize(CameraPosition - this->Position);

    //  if (GoalRotation == glm::vec3(0.0f, 0.0f, 1.0f)) return;

    glm::vec3 rotationAxis = glm::normalize(glm::cross(this->InitialRotation, GoalRotation));
    float rotationAngle = std::acos(glm::dot(glm::normalize(InitialRotation), glm::normalize(GoalRotation))) * 180.0f / static_cast<float>(M_PI);
    auto rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis);
    setRotation(rot);
    
}

bool Renderable::update_attacker(AppVulkanImpl* app, const glm::vec3& CameraPosition, float deltaTime)
{

    if (glm::distance(CameraPosition, this->Position) < 0.1f)
    {
        app->end_game();
        return false;
    }

    glm::vec3 GoalRotation = glm::normalize(CameraPosition - this->Position);

    glm::vec3 direction = glm::normalize(GoalRotation);

    glm::vec3 rotationAxis = glm::normalize(glm::cross(this->InitialRotation, GoalRotation));
    float rotationAngle = std::acos(glm::dot(glm::normalize(InitialRotation), glm::normalize(GoalRotation))) * 180.0f / static_cast<float>(M_PI);

    setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis));

    static const float attackerSpeed = 10.0f;
    float distance = attackerSpeed * deltaTime;

    setTranslation(glm::translate(glm::mat4(1.0f), glm::vec3(this->Position + distance * direction)));
    return true;

}

bool Renderable::intersect(glm::vec3& GunPosition, glm::vec3& Direction)
{

        glm::vec3 TargetDir = glm::normalize(this->Position - GunPosition);
        float dotProduct = glm::dot(TargetDir, Direction);
        if (dotProduct < 0.0f) return false;

        // t satisfies a quadratic
        double a = glm::dot(Direction, Direction);
        double b = 2 * glm::dot(Direction, GunPosition - this->Position);
        double c = glm::dot(GunPosition - Position, GunPosition - Position) - this->Radius * this->Radius;
        double discriminant = b * b - 4 * a * c;

        if (discriminant < 0)
        {
            return false;
        }

        return true;
    
}

void Renderable::update_light_source(float deltaTime)
{
    static const float rotationSpeed = static_cast<float>(glm::two_pi<float>()) / 10.0f; // Radians per second for a full rotation in 10 seconds

    float angle = rotationSpeed * deltaTime;
    this->Model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * this->Model;
    this->Position = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(Position, 1.0f);
}

void Renderable::setTranslation(glm::mat4 translation)
{
    this->Translation = translation;
    compute_model_matrix();
}
void Renderable::setRotation(glm::mat4 rotation)
{
    this->Rotation = rotation;
    compute_model_matrix();
}
void Renderable::setScale(glm::mat4 scale)
{
    this->Scale = scale;
    compute_model_matrix();
}

void Renderable::update_position(glm::vec3 position, glm::vec3 Front)
{
    this->Position = position;
    setTranslation(glm::translate(glm::mat4(1.0f), Position));

    glm::vec3 GoalRotation = Front;
    this->Direction = GoalRotation;

    glm::vec3 rotationAxis = glm::normalize(glm::cross(this->InitialRotation, GoalRotation));
    float rotationAngle = std::acos(glm::dot(glm::normalize(InitialRotation), glm::normalize(GoalRotation))) * 180.0f / static_cast<float>(M_PI);

    setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis));
}

void Renderable::update_swing(float time)
{
    glm::vec3 center = this->MeshHandle->SwingCenter;
    float radius = this->MeshHandle->SwingRadius;
    size_t size = this->MeshHandle->SwingAngles.size();

    int segment = static_cast<int>(std::floor(time));

    acc = segment % 2 == 0 ? true : false;

    float angle1 = this->MeshHandle->SwingAngles[segment % size];
    float angle2 = this->MeshHandle->SwingAngles[(segment + 1) % size];

    time -= std::floor(time);
    float acceleration = 2 * (angle2 - angle1);
    float angle;
    if (acc)
    {
        angle = angle1 + acceleration / 2.f * time * time;
    }
    else
    {
        float vo = acceleration;
        angle = angle1 + vo * time - acceleration / 2.f * time * time;
    }

   // if (acceleration == 0) return;
   // float rotationAngle =  * 180.0f / static_cast<float>(M_PI);
    this->Position = {center.x + radius*cos(glm::radians(angle)), center.y + radius*sin(glm::radians(angle)), 0.0f};

    this->MeshHandle->tail->object->setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(angle + 90.0f), glm::vec3(0.0f, 0.0f, 1.0f)));

    setTranslation(glm::translate(glm::mat4(1.0f), Position));

}

void Renderable::compute_animation(float time)
{
    int segment = static_cast<int>(std::floor(time));

    time -= std::floor(time);

    auto animation = this->MeshHandle->mesh.Animation;

    glm::mat4x3 R = {
            {animation[segment % animation.size()]},
            {animation[(segment + 1) % animation.size()]},
            {animation[(segment + 2) % animation.size()]},
            {animation[(segment + 3) % animation.size()]}
    };
    glm::vec4 T = glm::vec4(glm::pow(time, 3.0f), glm::pow(time, 2.0f), time, 1.0f);

    this->Position = glm::vec3(R * this->MeshHandle->mesh.B * T / 6.0f);
    setTranslation(glm::translate(glm::mat4(1.0f), Position));

    glm::vec3 T_d = glm::vec3(glm::pow(time, 2.0f), time, 1.0f);

    glm::vec3 GoalRotation = glm::vec3(glm::mat3(R * this->MeshHandle->mesh.B_d) * T_d) / 2.0f;
    this->Direction = GoalRotation;

    glm::vec3 rotationAxis = glm::normalize(glm::cross(this->InitialRotation, GoalRotation));
    float rotationAngle = std::acos(glm::dot(glm::normalize(InitialRotation), glm::normalize(GoalRotation))) * 180.0f / static_cast<float>(M_PI);

    setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis));
}

void Renderable::bind_materials(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
{
    for (int i = 0; i < this->MeshHandle->pipeline->pipelineLayout->descriptorSetLayout.size(); ++i)
    {
        if (!(this->MeshHandle->pipeline->pipelineLayout->descriptorSetLayout[i]->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            || this->MeshHandle->pipeline->pipelineLayout->descriptorSetLayout[i]->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE))
        {
           vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->MeshHandle->pipeline->pipelineLayout->layout, i, 1, &this->MeshHandle->pipeline->pipelineLayout->descriptorSetLayout[i]->descriptorSets[imageIndex], 0, nullptr);
        }
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->MeshHandle->pipeline->pipeline);
}

void Renderable::bind_mesh(VkCommandBuffer& commandBuffer)
{
    VkBuffer vertexBuffers[] = { this->MeshHandle->mesh.VertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, this->MeshHandle->mesh.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void Renderable::bind_texture(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
{
    int texturesSize = static_cast<int>(this->MeshHandle->textures.size());
    int imageFieldsSize = static_cast<int>(this->MeshHandle->image_fields.size());
    for (int i = 0; i < texturesSize; ++i)
    {
        std::shared_ptr<Texture>& texture = this->MeshHandle->textures[i];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->MeshHandle->pipeline->pipelineLayout->layout, static_cast<uint32_t>(this->MeshHandle->pipeline->pipelineLayout->descriptorSetLayout.size()) - texturesSize - imageFieldsSize + i, 1, &texture->descriptorSets[imageIndex], 0, nullptr);
    }
}

void Renderable::bind_image_fields(VkCommandBuffer& commandBuffer, uint32_t imageIndex)
{
    int descriptorSize = static_cast<uint32_t>(this->MeshHandle->pipeline->pipelineLayout->descriptorSetLayout.size());
    int imageFieldsSize = static_cast<int>(this->MeshHandle->image_fields.size());
    for (int i = 0; i < imageFieldsSize; ++i)
    {
        std::shared_ptr<Texture>& image_field = this->MeshHandle->image_fields[i];
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->MeshHandle->pipeline->pipelineLayout->layout, descriptorSize - imageFieldsSize + i, 1, &image_field->descriptorSets[imageIndex], 0, nullptr);
    }
}