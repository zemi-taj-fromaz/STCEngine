#pragma once

#include "VulkanInit.h"

#include <optional>
#include <array>
#include <deque>
#include <cmath>

#define M_PI 3.14159265358979323846

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR Capabilities; //Min MAx images supported in the swap chain, range of resolutions
    std::vector<VkSurfaceFormatKHR> Formats;    //Format and ColorSpace - Format = BGRAlpha, ColorSpace-neki kurac
    std::vector<VkPresentModeKHR> PresentModes; //MOST IMPORTANT- conditions for showing images to the screen
};

struct QueueFamilyIndices {
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    bool isComplete() {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
    }
};

struct SyncObjects
{
    VkSemaphore ImageAvailableSemaphore;
    VkSemaphore RenderFinishedSemaphore;
    VkSemaphore ComputeFinishedSemaphore;
    VkFence ComputeInFlightFence;
    VkFence InFlightFence;
};

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Color;
    glm::vec2 TexCoord;

    bool operator==(const Vertex& other) const {
        return Position == other.Position && Normal == other.Normal && Color == other.Color && TexCoord == other.TexCoord;
    }

    static VkVertexInputBindingDescription get_binding_description() {

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4); //<----------------------------------------------------------

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, Color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, TexCoord);


        return attributeDescriptions;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.Position) ^
                (hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.TexCoord) << 1);
        }
    };
}

struct CameraBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 pos;
};

struct Resolution
{
    glm::vec2 resolution;
};

struct ParameterUBO{
    float deltaTime;
};

struct Particle {
    glm::vec3 position;
    glm::vec2 velocity{ 0.0f, 0.0f };
    glm::vec4 color;

    static VkVertexInputBindingDescription get_binding_description() {

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Particle);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Particle, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Particle, color);

        return attributeDescriptions;
    }
};

struct NamedFunction
{
    std::function<void()> Function;
    std::string Name;
};

struct DeletionQueue
{
    //std::deque<std::function<void()>> deletors;
    std::deque<NamedFunction> deletors;

    void push_function(std::function<void()>&& function, std::string name) {
        deletors.push_back({function , name});
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            std::cout << (*it).Name << std::endl;
            (*it).Function(); //call the function
        }

        deletors.clear();
    }
};


//TODO ukloni ovu klasu
struct Material
{
    VkPipeline Pipeline;
    VkPipelineLayout PipelineLayout;
    int descriptorSetCount;
};


//TODO SOON TO BE GONE

struct SceneData {
    glm::vec4 fogColor{ 0.0f, 0.0f, 0.0f, 0.0f }; // w is for exponent
    glm::vec4 fogDistances{ 0.0f, 0.0f, 0.0f, 0.0f }; //x for min, y for max, zw unused.
    glm::vec4 ambientColor{ 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec4 sunlightDirection{ 0.0f, 0.0f, 0.0f, 0.0f }; //w for sun power
    glm::vec4 sunlightColor{ 0.0f, 0.0f, 0.0f, 0.0f };
    glm::vec4 sunPosition{ -500.0f, 400.0f, 350.0f, 0.0f };
};

struct ObjectData
{
    glm::mat4 Model;
    glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
};

// i ovo je useless? TODO
struct Object
{
    std::vector<ObjectData> Data;
    VkBuffer Buffer;
    VkDeviceMemory Memory;
    void* Mapped;
};

//I ovo? TODO
struct Scene
{
    SceneData Data;
    VkBuffer DataBuffer;
    VkDeviceMemory DataMemory;
    void* DataMapped;
};

struct UploadContext {
    VkFence UploadFence;
    VkCommandPool CommandPool;
    VkCommandBuffer CommandBuffer;
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






