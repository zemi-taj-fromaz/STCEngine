#pragma once

#include "VulkanInit.h"

#include "AppImpl.h"
#include "PipelineBuilderStateMachine.h"
#include "Mesh.h"
#include "Camera.h"
#include "RenderObject.h"
#include "RenderParticle.h"
#include "RenderLight.h"
#include "Texture.h"
#include "Descriptor.h"


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

enum class AppType
{
    APP_TYPE_2D,
    APP_TYPE_3D
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

    inline void set_frame_buffer_resized() { m_FramebufferResized = true; } // TODO PROPAGACIJA FUNKCIJE AKTIVNOM LAYERU }
    inline void set_camera_offset(float offset) { float zoomSpeed = 1.f; camera_offset -= offset * zoomSpeed; }


    inline glm::vec2 get_mouse_position() { return this->m_MousePosition; }
    inline void set_mouse_position(glm::vec2 mousePosition) { this->m_MousePosition = mousePosition; }

    void fire_gun();

    inline void process_mouse_movement(float xoffset, float yoffset) { this->m_LayerStack[m_ActiveLayer]->m_Camera.process_mouse_movement(xoffset, yoffset); }
    inline void set_field_of_view(float yoffset) { this->m_LayerStack[m_ActiveLayer]->m_Camera.set_field_of_view(yoffset); }

    Camera get_camera() { return this->m_LayerStack[m_ActiveLayer]->m_Camera; }
    VkExtent2D get_swapchain_extent() { return this->m_SwapChainExtent; }

    float get_total_time() { return totalTime; }
    float get_delta_time() { return deltaTime; }
    glm::vec2 get_resolution() { return glm::vec2(width,height); }
    size_t get_particles_size() { return particles.size(); }
    std::vector<std::shared_ptr<Renderable>>& get_renderables() { return m_Renderables; }
    std::vector<std::shared_ptr<Renderable>>& get_attackers() { return m_Attackers; }
    std::vector<std::shared_ptr<RenderLight>>& get_point_lights() { return m_PointLights; }
    std::vector<std::shared_ptr<RenderLight>>& get_flash_lights() { return m_FlashLights; }
    std::shared_ptr<RenderLight>& get_global_light() { return m_GlobalLight; }
    std::shared_ptr<RenderLight>& get_camera_light() { return m_CameraLight; }
    void create_mesh(MeshWrapper& mesh);

private:
    void create_instance();
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device(); // - IMPROVEMENT? : Device selection algorithm ( VkPhysicalDeviceMemoryProperties)
    void create_logical_device();
    void create_swapchain();
 //   void create_image_views();
    void create_render_pass();
    void create_descriptor_set_layout(std::shared_ptr<Layer>& layer); //LAYER
    void create_graphics_pipeline(std::shared_ptr<Layer>& layer); //LAYER
    void create_framebuffers(); 
    void create_depth_resources();

    void create_texture_image(Texture& texture);
    void create_cubemap(Texture& texture);
   // void create_texture_image_view();
    void create_texture_sampler();

    void load_model(std::shared_ptr<Layer>& layer); //LAYER

    void create_buffers(std::shared_ptr<Layer>& layer); //LAYER

    void create_descriptors(std::shared_ptr<Layer>& layer); //LAYER

    void create_commands();

    void create_sync_objects();


    void draw_frame(std::shared_ptr<Layer>& layer);


    void init_imgui();

private:

    std::vector<std::shared_ptr<Renderable>> m_Attackers;

    int m_ActiveLayer{ 0 };

   // void AppVulkanImpl::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

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

    void Recreate_swapchain();
    void cleanup_swap_chain();

    uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, unsigned int arrayLayers = 1);

    VkCommandBuffer  begin_single_time_commands();
    void  end_single_time_commands(VkCommandBuffer commandBuffer);

    void transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, unsigned int layerCount = 1);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, unsigned int layerCount = 1);
    VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, bool cubeMap = false);

    VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat find_depth_format();
    bool has_stencil_component(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void upload_mesh(Mesh& mesh);

  //  void update_transform_matrices();
    
    void draw_objects(VkCommandBuffer commandBuffer, std::vector<std::shared_ptr<Renderable>> renderables, uint32_t imageIndex);
    void draw_compute(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    VkDescriptorSetLayoutBinding create_descriptor_set_layout_binding(int binding, int count, VkDescriptorType type, VkShaderStageFlagBits shaderStageFlag);
    VkWriteDescriptorSet write_descriptor_set(VkDescriptorSet set, int binding, VkDescriptorType type, const VkDescriptorBufferInfo& bufferInfo);
    VkWriteDescriptorSet write_descriptor_image(VkDescriptorSet set, int binding, VkDescriptorType type, const VkDescriptorImageInfo& imageInfo);
    VkDescriptorSetLayoutCreateInfo create_layout_info(VkDescriptorSetLayoutBinding* bindings, size_t size);

    VkDescriptorSetAllocateInfo create_descriptor_alloc_info(VkDescriptorSetLayout* layouts, size_t size);


    void create_mesh_obj(Mesh& mesh, bool illuminated, std::shared_ptr<Texture> texture, std::optional<std::string> animation = std::nullopt);
    void create_mesh(std::vector<Vertex> vertices, Mesh& mesh, bool illuminated, std::optional<int> textureIndex, std::optional<std::string> animation);

    VkDescriptorImageInfo create_descriptor_image_info(VkSampler sampler, VkImageView imageView);
    VkDescriptorBufferInfo create_descriptor_buffer_info(VkBuffer buffer, VkDeviceSize size,VkDeviceSize offset = 0);

private:

    std::vector<std::shared_ptr<RenderLight>> m_PointLights;
    std::vector<std::shared_ptr<RenderLight>> m_FlashLights;
    std::shared_ptr<RenderLight> m_GlobalLight;
    std::shared_ptr<RenderLight> m_CameraLight;

    std::vector<Particle> particles;

    bool isInitialized{ false };

    static const AppType appType{ AppType::APP_TYPE_2D };

    static bool s_ImGuiEnabled;

    GLFWwindow* m_Window;
    const uint32_t width = 1280;
    const uint32_t height = 720;
    int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t m_CurrentFrame{ 0 };
    bool m_FramebufferResized = false;

    PipelineBuilder m_PipelineBuilder;
    DeletionQueue m_DeletionQueue;
    

#ifdef NDEBUG
    const bool ENABLED_VALIDATION_LAYERS = false;
#else
    const bool ENABLED_VALIDATION_LAYERS = true;
#endif

private:
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    
    const std::vector<const char*> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkQueue m_GraphicsQueue;
    VkQueue m_ComputeQueue;
    VkQueue m_PresentQueue;
    
    
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;
    std::vector<VkImageView> m_SwapChainImageViews;
    
    VkRenderPass m_RenderPass;
    
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<VkCommandBuffer> m_ComputeCommandBuffers;
    std::vector<SyncObjects> m_SyncObjects;

 
    VkDescriptorPool m_DescriptorPool;
  
    VkSampler m_TextureSampler;

    VkImage m_DepthImage;
    VkDeviceMemory m_DepthImageMemory;
    VkImageView m_DepthImageView;

    std::vector<std::shared_ptr<Renderable>> m_Renderables;
    RenderObject* m_SkyboxObj{ nullptr };

    glm::vec2 m_MousePosition{ width /2.0f, height / 2.0f };
    float camera_offset = 10.0f; 

    UploadContext m_UploadContext;
    VkDescriptorPool m_ImguiPool;


    float totalTime;
    float deltaTime;
};


