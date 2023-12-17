#include "Renderable.h"
#include <random>

class RenderParticle : public Renderable
{
public:
    RenderParticle(MeshWrapper* meshHandle, bool isSkybox) : Renderable(meshHandle, isSkybox)
    {
    }

    RenderParticle(MeshWrapper* meshHandle) : Renderable(meshHandle, false)
    {
    }

    RenderParticle(const RenderParticle& renderObject);
    ~RenderParticle(){}

    void update(float time, glm::vec3& cameraPosition);

    std::shared_ptr<RenderObject> generator;
private:
    void respawn();

    static  std::random_device rd;
    static  std::mt19937 gen;
    static std::uniform_real_distribution<float> m_DistributionVelocity;
    static std::uniform_real_distribution<float> m_DistributionPosition;
    static std::uniform_real_distribution<float> m_DistributionLife;

    static const glm::vec3 SpawnCoord;

    float LifeRemaining{ m_DistributionLife(gen) };
    float TotalLife{ LifeRemaining };

    glm::vec3 Velocity{ 0.0f, 0.0f, 0.0f }; //m_DistributionVelocity(gen), m_DistributionVelocity(gen), 0.0f};
    glm::vec3 Position{ m_DistributionPosition(gen), m_DistributionPosition(gen), 0.0f };
};