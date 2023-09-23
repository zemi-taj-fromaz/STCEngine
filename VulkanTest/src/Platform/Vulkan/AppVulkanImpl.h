#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "AppImpl.h"

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

class AppVulkanImpl : public AppImpl
{
public:
    AppVulkanImpl();
    ~AppVulkanImpl();

    virtual void InitializeWindow() override;
    virtual void InitializeApp() override;
    virtual void MainLoop() override;
    virtual void Cleanup() override;

private:
    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice(); // - IMPROVEMENT? : Device selection algorithm ( VkPhysicalDeviceMemoryProperties)
    void CreateLogicalDevice();
    void CreateSwapChain();

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


private:
    GLFWwindow* m_Window;
    const uint32_t m_Width = 1280;
    const uint32_t m_Height = 720;

#ifdef NDEBUG
    const bool ENABLED_VALIDATION_LAYERS = false;
#else
    const bool ENABLED_VALIDATION_LAYERS = true;
#endif

private:
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkSurfaceKHR m_Surface;
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;

};


