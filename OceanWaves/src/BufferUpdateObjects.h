#pragma once
#include <Engine.h>

struct ObjectData
{
    glm::mat4 Model;
    glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct GlobalLight
{
    glm::vec4 ambientColor;
    glm::vec4 diffColor;
    glm::vec4 specColor;
    glm::vec4 direction;
};

struct WindowDims
{
    uint32_t W;
    uint32_t H;
};