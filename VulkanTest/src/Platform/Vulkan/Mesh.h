#pragma once

#include "VulkanInit.h"
#include "HelperObjects.h"

enum class MeshType
{
    None = 0,
    Sphere = 1,
    Cube = 2,
    Quad = 3,
    Line = 4,
    Terrain = 5,
    Plain = 6
};

struct Mesh
{
    Mesh()
    {}

    Mesh(std::string filename) : Filename(filename)
    {}

    Mesh(MeshType meshType) : meshType(meshType)
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
        TextureSets = mesh.TextureSets;
        Animated = mesh.Animated;
        Filename = mesh.Filename;
        meshType = mesh.meshType;
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
    MeshType meshType{ MeshType::None };

    std::vector<VkDescriptorSet> TextureSets; //texture defaulted to null

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
    bool load_sphere(bool illuminated, bool textured);

    bool load_cube(bool illuminated, bool textured);
    bool load_quad(bool illuminated, bool textured);
    bool load_line(bool illuminated, bool textured);
    bool load_terrain(bool illuminated, bool textured);
    bool load_plain(bool illuminated, bool textured, uint32_t tileSize, float tileLength);

    bool load(bool illuminated, bool textured);

    bool load_animation(std::string filename);


    const int ARENA_SIZE{ 800 };

    static std::vector<std::vector<float>> heightMap;
};
