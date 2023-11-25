#pragma once

#include "VulkanInit.h"
#include "HelperObjects.h"

struct Mesh
{
    Mesh()
    {}

    Mesh(std::string filename) : Filename(filename)
    {}

    Mesh(const Mesh& mesh)
    {
        Vertices = mesh.Vertices;
        Indices = mesh.Indices;
        VertexBuffer = mesh.VertexBuffer;
        IndexBuffer = mesh.IndexBuffer;
        VertexBufferMemory = mesh.VertexBufferMemory;
        IndexBufferMemory = mesh.IndexBufferMemory;
        Animation = mesh.Animation;
        Animated = mesh.Animated;
        Filename = mesh.Filename;
    }

    Mesh operator=(const Mesh& mesh)
    {
        return Mesh(mesh);
    }

    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    VkBuffer VertexBuffer;
    VkBuffer IndexBuffer;
    VkDeviceMemory VertexBufferMemory;
    VkDeviceMemory IndexBufferMemory;
    bool Animated{ false };
    std::string Filename;

    std::vector<glm::vec3> Animation;

    const glm::mat4 B = {
        {-1, 3, -3, 1},
        {3, -6, 3, 0},
        {-3, 0, 3, 0},
        {1, 4, 1, 0}
    };

    const glm::mat3x4 B_d = {
        {-1, 3, -3, 1},
        {2, -4, 2, 0},
        {-1, 0, 1, 0}
    };

    const std::string MODEL_PATH = "resources/models/";
    const std::string ANIMATION_PATH = "resources/animations/";


    bool load_from_obj(bool illuminated, bool texture = false);
    bool load_animation(std::string filename);
};
