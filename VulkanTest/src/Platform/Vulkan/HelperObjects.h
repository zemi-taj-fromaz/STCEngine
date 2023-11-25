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



struct Material
{
    std::vector<VkDescriptorSet> TextureSets; //texture defaulted to null
    VkPipeline Pipeline;
    VkPipelineLayout PipelineLayout;
};

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
};

struct Object
{
    std::vector<ObjectData> Data;
    VkBuffer Buffer;
    VkDeviceMemory Memory;
    void* Mapped;
};

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





