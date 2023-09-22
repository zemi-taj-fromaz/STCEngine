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
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();

private:
    GLFWwindow* m_Window;
    const uint32_t m_Width = 1280;
    const uint32_t m_Height = 720;

private:
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    const std::vector<const char*> m_ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    const bool ENABLED_VALIDATION_LAYERS = false;
#else
    const bool ENABLED_VALIDATION_LAYERS = true;
#endif
};


