#pragma once
#include <Engine.h>

struct ObjectData
{
    glm::mat4 Model;
    glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
};


struct WindowDims
{
    uint32_t W;
    uint32_t H;
};

struct PointLight
{
    glm::vec4 position;
    glm::vec4 ambientColor;
    glm::vec4 diffColor;
    glm::vec4 specColor;
    glm::vec4 clq;

    int size;
};


struct GlobalLight
{
    glm::vec4 ambientColor;
    glm::vec4 diffColor;
    glm::vec4 specColor;
    glm::vec4 direction;
};



struct FlashLight
{
    glm::vec4 position;
    glm::vec4 ambientColor;
    glm::vec4 diffColor;
    glm::vec4 specColor;
    glm::vec4 clq;

    glm::vec4 direction;
    float innerCutoff;
    float outerCutoff;

    int size;
};

struct Material
{
    // material parameters
    glm::vec3  albedo;
    float metallic;
    float roughness;
    float ao;
};