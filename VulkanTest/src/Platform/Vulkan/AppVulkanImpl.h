#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "AppImpl.h"

#include <array>
#include <vector>
#include <iostream>
#include <optional>
#include <chrono>

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
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 TexCoord;

    bool operator==(const Vertex& other) const {
        return Position == other.Position && Color == other.Color && TexCoord == other.TexCoord;
    }

    static VkVertexInputBindingDescription get_binding_description() {

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> get_attribute_descriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, TexCoord);


        return attributeDescriptions;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.Position) ^
                (hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.TexCoord) << 1);
        }
    };
}

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class AppVulkanImpl : public AppImpl
{
public:
    AppVulkanImpl();
    ~AppVulkanImpl();

    virtual void initialize_window() override;
    virtual void initialize_app() override;
    virtual void main_loop() override;
    virtual void cleanup() override;

    inline void set_frame_buffer_resized() { m_FramebufferResized = true;  }

private:
    void create_instance();
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device(); // - IMPROVEMENT? : Device selection algorithm ( VkPhysicalDeviceMemoryProperties)
    void create_logical_device();
    void create_swapchain();
    void create_image_views();
    void create_render_pass();
    void create_descriptor_set_layout();
    void create_graphics_pipeline();
    void create_framebuffers();
    void create_command_pool();
    void create_depth_resources();
    void create_texture_image();
    void create_texture_image_view();
    void create_texture_sampler();
    void load_model();
    void create_texture_sample();
    void create_index_buffer();
    void create_uniform_buffers();
    void create_descriptor_pool();
    void create_descriptor_sets();
    void create_command_buffers();
    void create_sync_objects();


    void draw_frame();

private:
    bool check_validation_layer_support();
    std::vector<const char*> get_required_extensions();
    VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pcreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool is_device_suitable(VkPhysicalDevice device); // - IMPROVEMENT? - extra features & properties requiremens ( VkPhysicalDeviceMemoryProperties)
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);
    SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);

    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkShaderModule create_shader_module(const std::vector<uint32_t>& code);
    void record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void Recreate_swapchain();
    void cleanup_swap_chain();

    uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void update_uniform_buffer(uint32_t currentImage);
    void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    VkCommandBuffer  begin_single_time_commands();
    void  end_single_time_commands(VkCommandBuffer commandBuffer);

    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat find_depth_format();
    bool has_stencil_component(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

private:
    GLFWwindow* m_Window;
    const uint32_t m_Width = 1280;
    const uint32_t m_Height = 720;
    const int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t m_CurrentFrame{ 0 };
    bool m_FramebufferResized = false;

    const std::string MODEL_PATH =   "resources/models/viking_room.obj";
    const std::string TEXTURE_PATH = "resources/textures/viking_room.png";
    
    std::vector<Vertex> m_Vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f,1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f,1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    };

    std::vector<uint32_t> m_Indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
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
    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_GraphicsPipeline;
    
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<SyncObjects> m_SyncObjects;

    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;
    VkBuffer m_IndexBuffer;
    VkDeviceMemory m_IndexBufferMemory;
    
    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;
    std::vector<void*> m_UniformBuffersMapped;

    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;

    VkImage m_TextureImage;
    VkDeviceMemory m_TextureImageMemory;
    VkImageView m_TextureImageView;
    VkSampler m_TextureSampler;

    VkImage m_DepthImage;
    VkDeviceMemory m_DepthImageMemory;
    VkImageView m_DepthImageView;
    

};


