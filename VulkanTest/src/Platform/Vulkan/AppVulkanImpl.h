#pragma once

#include "VulkanInit.h"

#include "AppImpl.h"
#include "PipelineBuilderStateMachine.h"
#include "Mesh.h"
#include "Camera.h"
#include "RenderObject.h"
#include "Texture.h"


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

    inline void set_frame_buffer_resized() { m_FramebufferResized = true;  }
    inline void set_camera_offset(float offset) { float zoomSpeed = 1.f; camera_offset -= offset * zoomSpeed; update_camera_buffer(); }


    inline glm::vec2 get_mouse_position() { return this->m_MousePosition; }
    inline void set_mouse_position(glm::vec2 mousePosition) { this->m_MousePosition = mousePosition; }

    inline void process_mouse_movement(float xoffset, float yoffset) { this->m_Camera.process_mouse_movement(xoffset, yoffset); }
    inline void set_field_of_view(float yoffset) { this->m_Camera.set_field_of_view(yoffset); };

    bool m_SkyboxOn{ false };

private:
    void create_instance();
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device(); // - IMPROVEMENT? : Device selection algorithm ( VkPhysicalDeviceMemoryProperties)
    void create_logical_device();
    void create_swapchain();
 //   void create_image_views();
    void create_render_pass();
    void create_descriptor_set_layout();
    void create_graphics_pipeline();
    void create_framebuffers();
    void create_depth_resources();

    void create_texture_image(Texture& texture);
    void create_cubemap(Texture& texture);
   // void create_texture_image_view();
    void create_texture_sampler();

    void load_model();

    void create_buffers();

    void create_descriptors();

    void create_commands();

    void create_sync_objects();


    void draw_frame(float deltaTime);


    void init_imgui();

private:
    void AppVulkanImpl::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

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

    void update_camera_buffer();
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
    void create_material(Material& material, VkPipeline pipeline, VkPipelineLayout layout);
    void create_material(Material& material, VkPipeline pipeline, VkPipelineLayout layout, std::vector<VkDescriptorSet> textureSet);

  //  void update_transform_matrices();
    
    void draw_objects(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects, uint32_t imageIndex);
    VkDescriptorSetLayoutBinding create_descriptor_set_layout_binding(int binding, int count, VkDescriptorType type, VkShaderStageFlagBits shaderStageFlag);
    VkWriteDescriptorSet write_descriptor_set(VkDescriptorSet set, int binding, VkDescriptorType type, const VkDescriptorBufferInfo& bufferInfo);
    VkWriteDescriptorSet write_descriptor_image(VkDescriptorSet set, int binding, VkDescriptorType type, const VkDescriptorImageInfo& imageInfo);
    VkDescriptorSetLayoutCreateInfo create_layout_info(VkDescriptorSetLayoutBinding* bindings, size_t size);

    VkDescriptorSetAllocateInfo create_descriptor_alloc_info(VkDescriptorSetLayout* layouts, size_t size);

    void create_mesh(Mesh& mesh, bool illuminated, bool textured, std::optional<std::string> animation = std::nullopt);
  //  RenderObject create_render_object(std::string meshName, std::string materialName);

    VkDescriptorImageInfo create_descriptor_image_info(VkSampler sampler, VkImageView imageView);

    VkDescriptorBufferInfo create_descriptor_buffer_info(VkBuffer buffer, VkDeviceSize size,VkDeviceSize offset = 0);


private:

    static const AppType appType{ AppType::APP_TYPE_2D };

    static bool s_ImGuiEnabled;

    GLFWwindow* m_Window;
    const uint32_t m_Width = 1280;
    const uint32_t m_Height = 720;
    int MAX_FRAMES_IN_FLIGHT = 2;
    uint32_t m_CurrentFrame{ 0 };
    bool m_FramebufferResized = false;

    PipelineBuilder m_PipelineBuilder;
    DeletionQueue m_DeletionQueue;

    
    std::vector<Vertex> m_Vertices;

    std::vector<uint32_t> m_Indices;

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
    VkQueue m_PresentQueue;
    
    
    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;
    std::vector<VkImageView> m_SwapChainImageViews;
    
    VkRenderPass m_RenderPass;
    VkDescriptorSetLayout m_SceneSetLayout;

    VkPipelineLayout m_TexturePipelineLayout;
    VkPipeline m_TexturePipeline;

    VkPipelineLayout m_PlainPipelineLayout;
    VkPipeline m_PlainPipeline;

    VkPipeline m_IlluminatedPipeline;

    VkPipeline m_CubemapPipeline;
    VkPipelineLayout m_CubemapPipelineLayout;
    
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<SyncObjects> m_SyncObjects;

    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;
    VkBuffer m_IndexBuffer;
    VkDeviceMemory m_IndexBufferMemory;
    
    VkBuffer m_CameraBuffer;
    VkDeviceMemory m_CameraBufferMemory;
    void* m_CameraBufferMapped;

    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_SceneSets;
    std::vector<VkDescriptorSet> m_ObjectSets;
    std::vector<VkDescriptorSet> m_TextureSets;

    VkSampler m_TextureSampler;

    VkSampler m_CubeSampler;

    VkImage m_DepthImage;
    VkDeviceMemory m_DepthImageMemory;
    VkImageView m_DepthImageView;

    std::vector<RenderObject> m_RenderObjects;

    //float x_offset{ 0.0f };
    //float y_offset{ 0.0f };

    Camera m_Camera;

    glm::vec2 m_MousePosition{ m_Width /2.0f, m_Height / 2.0f };

    float camera_offset = 150.0f; 

    Scene m_Scene;

    std::vector<Object> m_Objects;
    VkDescriptorSetLayout m_ObjectSetLayout;
    VkDescriptorSetLayout m_TextureSetLayout;
    VkDescriptorSetLayout m_CubemapSetLayout;

    UploadContext m_UploadContext;

    Mesh m_Jet          { "fighter_jet.obj" };
    Mesh m_Panda        { "panda.obj" };
    Mesh m_Cat          { "cat.obj" };
    Mesh m_Skybox       { "skybox.obj" };
    Mesh m_TextureTest  { "texture.obj" };
    Mesh m_Spiral       { "spiral.obj" };

    VkDescriptorPool m_ImguiPool;

    RenderObject m_SkyboxObj;

    Material m_IlluminateMaterial;
    Material m_SkyboxMaterial;
    Material m_TextureMaterial;
    Material m_PlainMaterial;

    Texture m_FighterJetMain    { "BODYMAINCOLORCG.png"};
    Texture m_FighterJetCamo    {"BODYCAMBUMPCG.png"   };
    Texture m_SkyboxTexture     {"stormydays/"             };
    Texture m_Statue            {"statue.jpg"          };

};


