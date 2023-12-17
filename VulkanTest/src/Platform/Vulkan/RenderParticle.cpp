#pragma once

#include "RenderObject.h"

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "RenderParticle.h"


std::random_device RenderParticle::rd{};
std::mt19937 RenderParticle::gen{ RenderParticle::rd() };
const glm::vec3 RenderParticle::SpawnCoord{ 0.0f, 0.0f, 0.0f };
std::uniform_real_distribution<float> RenderParticle::m_DistributionVelocity{ -20.0f, 20.0f };
std::uniform_real_distribution<float> RenderParticle::m_DistributionPosition{ -20.0f, 20.0f };
std::uniform_real_distribution<float> RenderParticle::m_DistributionLife{ 0.5f, 2.0f };

//RenderObject::RenderObject(const RenderObject& renderObject)
//{
//    MeshHandle = renderObject.MeshHandle;
//    isSkybox = renderObject.isSkybox;
//    Model = renderObject.Model;
//    Translation = renderObject.Translation;
//    Rotation = renderObject.Rotation;
//    Scale = renderObject.Scale;
//}
//
//RenderParticle::RenderParticle(const RenderParticle& renderObject)
//{
//    MeshHandle = renderObject.MeshHandle;
//    isSkybox = renderObject.isSkybox;
//    Model = renderObject.Model;
//    Translation = renderObject.Translation;
//    Rotation = renderObject.Rotation;
//    Scale = renderObject.Scale;
//    Life = renderObject.Life;
//    TotalLife = renderObject.TotalLife;
//    Velocity = renderObject.Velocity;
//    Position = renderObject.Position;
//
//}

void RenderParticle::respawn()
{
    glm::vec3 position = this->generator->get_position();

    std::normal_distribution<float> distributionX(position.x, 0.1f);
    std::normal_distribution<float> distributionY(position.y, 0.1f);
    std::normal_distribution<float> distributionZ(position.z, 0.1f);

    glm::vec3 direction = this->generator->get_direction();

    std::normal_distribution<float> distributionVelX(-direction.x, 0.1f);
    std::normal_distribution<float> distributionVelY(-direction.y, 0.1f);
    std::normal_distribution<float> distributionVelZ(-direction.z, 0.1f);


    std::normal_distribution<float> distributionLife(1.0f, 0.2f);


    this->Position = glm::vec3(distributionX(gen), distributionY(gen), distributionZ(gen));
    this->Velocity = glm::vec3(distributionVelX(gen), distributionVelY(gen), distributionVelZ(gen));
    this->LifeRemaining = distributionLife(gen);
    this->TotalLife = LifeRemaining;
    
}

void RenderParticle::update(float time, glm::vec3& CameraPosition)
{
    if (this->LifeRemaining < time)
    {
        this->respawn();
    }
    else
    {
        this->LifeRemaining -= time;
        this->Position += this->Velocity * time;
    }

    if (this->LifeRemaining > this->TotalLife)
    {
        std::cout << "?" << std::endl;
        throw std::runtime_error("Life remaiing greater than total particle life");
    }

    setTranslation(glm::translate(glm::mat4(1.0f), this->Position));

    glm::vec3 GoalRotation = glm::normalize(CameraPosition - this->Position);

  //  if (GoalRotation == glm::vec3(0.0f, 0.0f, 1.0f)) return;

    glm::vec3 rotationAxis = glm::normalize(glm::cross(this->InitialRotation, GoalRotation));

    float rotationAngle = std::acos(glm::dot(glm::normalize(this->InitialRotation), glm::normalize(GoalRotation))) * 180.0f / static_cast<float>(M_PI);
    setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis));

    float lifeTimePercent = this->LifeRemaining / this->TotalLife;
    setScale(glm::scale(glm::scale(glm::mat4(1.0f), glm::vec3(lifeTimePercent)), glm::vec3(0.1f)));
}
