#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "AppImpl.h"

#include <array>
#include <vector>
#include <iostream>
#include <optional>

//The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the resolution of the window that we're drawing to in pixels 
//ako je currentExtnet = MAX -> odabiremo rezoluciju koja odgovara prozoru
/*
GLFW uses two units when measuring sizes: pixels and screen coordinates. 
For example, the resolution {WIDTH, HEIGHT} that we specified earlier when creating the window is measured in screen coordinates.
But Vulkan works with pixels, so the swap chain extent must be specified in pixels as well.
*/
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
    glm::vec2 Position;
    glm::vec3 Color;

    static VkVertexInputBindingDescription GetBindingDescription() {

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Color);

        return attributeDescriptions;
    }
};

class AppVulkanImpl : public AppImpl
{
public:
    AppVulkanImpl();
    ~AppVulkanImpl();

    virtual void InitializeWindow() override;
    virtual void InitializeApp() override;
    virtual void MainLoop() override;
    virtual void Cleanup() override;

    inline void SetFrameBufferResized() { m_FramebufferResized = true;  }

private:
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice(); // - IMPROVEMENT? : Device selection algorithm ( VkPhysicalDeviceMemoryProperties)
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateVertexBuffer();
    void CreateCommandBuffers();
    void CreateSyncObjects();


    void DrawFrame();

private:
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool IsDeviceSuitable(VkPhysicalDevice device); // - IMPROVEMENT? - extra features & properties requiremens ( VkPhysicalDeviceMemoryProperties)
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void RecreateSwapChain();
    void CleanupSwapChain();

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


private:
    GLFWwindow* m_Window;
    const uint32_t m_Width = 1280;
    const uint32_t m_Height = 720;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t m_CurrentFrame{ 0 };
    bool m_FramebufferResized = false;
    
    const std::vector<Vertex> m_Vertices = {
        {{0.0f, -0.5f}, {0.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

#ifdef NDEBUG
    const bool ENABLED_VALIDATION_LAYERS = false;
#else
    const bool ENABLED_VALIDATION_LAYERS = true;
#endif

private:
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    
    const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device;
    
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    
    VkSurfaceKHR m_Surface;
    
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;
    std::vector<VkImageView> m_SwapChainImageViews;
    
    VkRenderPass m_RenderPass;
    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_GraphicsPipeline;
    
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<SyncObjects> m_SyncObjects;

    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;
    
    

};


