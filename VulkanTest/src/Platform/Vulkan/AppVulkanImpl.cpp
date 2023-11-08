#include "AppVulkanImpl.h"


#include <iostream>
#include <algorithm>
#include <set>
#include <shaderc/shaderc.hpp>
#include <unordered_map>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

bool AppVulkanImpl::s_ImGuiEnabled = false;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        // Message is important enough to shows
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }


    return VK_FALSE;
}

AppVulkanImpl::AppVulkanImpl()
{
}

AppVulkanImpl::~AppVulkanImpl()
{
}

void AppVulkanImpl::initialize_window()
{

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    //// Get the primary monitor (screen)
    //GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    //// Get the mode of the primary monitor
    //const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    m_Window = glfwCreateWindow(m_Width, m_Height, "Vulkan Fullscreen", nullptr, NULL);
    glfwSetWindowUserPointer(m_Window, this);

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
        app->set_frame_buffer_resized();
        });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
        app->set_field_of_view(static_cast<float>(yoffset));
        });


    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
        glm::vec2 mousePosition = app->get_mouse_position();

        app->set_mouse_position({ static_cast<float>(xpos), static_cast<float>(ypos) });

        float xoffset = static_cast<float>(xpos) - mousePosition.x;
        float yoffset = mousePosition.y - static_cast<float>(ypos); // reversed since y-coordinates range from bottom to top

        const float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        app->process_mouse_movement(xoffset, yoffset);
       // app->process_mouse_movement(0.0f, 0.0f);
        });
}

void AppVulkanImpl::initialize_app()
{
    create_instance();
    setup_debug_messenger();
    create_surface();
    pick_physical_device();
    create_logical_device();
    create_swapchain();
    create_render_pass();

    create_descriptor_set_layout();
    create_graphics_pipeline();

    initialize_commands();
    create_depth_resources();
    create_framebuffers();

    create_texture_sampler();

    load_model();

    initialize_buffers();
    initialize_descriptors();
    create_sync_objects();

    if (AppVulkanImpl::s_ImGuiEnabled)
    {
        init_imgui();
    }
}

void AppVulkanImpl::main_loop()
{
    float time = (float)glfwGetTime();
    while (!glfwWindowShouldClose(m_Window)) {

        glfwPollEvents();
        
        float currTime = (float)glfwGetTime();
        float deltaTime = currTime - time;
        time = currTime;

        float cameraSpeed = 100.0f;

        // Check for key presses
        if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS) {
            m_Camera.Position += cameraSpeed * deltaTime * m_Camera.Front;
        }
     

        if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS) {
            m_Camera.Position += cameraSpeed * deltaTime * m_Camera.Right;

        }
  
        if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS) {
            m_Camera.Position -= cameraSpeed * deltaTime * m_Camera.Front;

        }
     
        if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS) {
            m_Camera.Position -= cameraSpeed * deltaTime * m_Camera.Right;

        }

        if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;

        }

        if (s_ImGuiEnabled)
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();

            ImGui::NewFrame();

            //imgui commands
            ImGui::ShowDemoWindow();
        }
        //imgui new frame

       

        draw_frame(deltaTime);
    }
    vkDeviceWaitIdle(m_Device);
}

void AppVulkanImpl::cleanup()
{
    cleanup_swap_chain();

    m_DeletionQueue.flush();


    //---------------------------------------------------------------------

    vkDestroyDevice(m_Device, nullptr);
    if (ENABLED_VALIDATION_LAYERS) {
        destroy_debug_utils_messenger_ext(m_Instance, m_DebugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);

    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//--------------------------------PRIVATE METHODS--------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void AppVulkanImpl::create_instance()
{
    if (ENABLED_VALIDATION_LAYERS && !check_validation_layer_support()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());


    std::vector<const char*> availableExtensionNames;
    std::for_each(availableExtensions.begin(), availableExtensions.end(), [&availableExtensionNames](VkExtensionProperties extensionProperties) {

        const char* extensionName = new char[strlen(extensionProperties.extensionName) + 1];
        strcpy_s(const_cast<char*>(extensionName),strlen(extensionProperties.extensionName) +1, & extensionProperties.extensionName[0]);

        availableExtensionNames.push_back(extensionName);
    });
    std::vector<const char*> requiredExtensions(get_required_extensions());

    
    std::sort(availableExtensionNames.begin(), availableExtensionNames.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
    std::sort(requiredExtensions.begin(), requiredExtensions.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
    
    if (!std::includes(availableExtensionNames.begin(), availableExtensionNames.end(), requiredExtensions.begin(), requiredExtensions.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; }))
    {
        throw std::runtime_error("Not all required extensions available");
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugcreateInfo{};
    if (ENABLED_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

        populate_debug_messenger_create_info(debugcreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugcreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void AppVulkanImpl::setup_debug_messenger()
{
    if (!ENABLED_VALIDATION_LAYERS) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populate_debug_messenger_create_info(createInfo);

    if (create_debug_utils_messenger_ext(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }

}

void AppVulkanImpl::create_surface() {
    if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void AppVulkanImpl::pick_physical_device() 
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (is_device_suitable(device)) {
            m_PhysicalDevice = device;
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

}

void AppVulkanImpl::create_logical_device()
{
    QueueFamilyIndices indices = find_queue_families(m_PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queuecreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queuecreateInfo{};
        queuecreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queuecreateInfo.queueFamilyIndex = queueFamily;
        queuecreateInfo.queueCount = 1;
        queuecreateInfo.pQueuePriorities = &queuePriority;
        queuecreateInfos.push_back(queuecreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queuecreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queuecreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());;
    createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

    if (ENABLED_VALIDATION_LAYERS) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_Device, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, indices.PresentFamily.value(), 0, &m_PresentQueue);
}

void AppVulkanImpl::create_swapchain()
{
    SwapChainSupportDetails swapChainSupport = query_swap_chain_support(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapChainSupport.Formats);
    VkPresentModeKHR presentMode = choose_swap_present_mode(swapChainSupport.PresentModes);
    VkExtent2D extent = choose_swap_extent(swapChainSupport.Capabilities);

    uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;

    MAX_FRAMES_IN_FLIGHT = imageCount;

    if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) {
        imageCount = swapChainSupport.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; //This is always 1 unless you are developing a stereoscopic 3D application.
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = find_queue_families(m_PhysicalDevice);
    uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    if (indices.GraphicsFamily != indices.PresentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());
    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;

    m_SwapChainImageViews.resize(m_SwapChainImages.size());
    for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
        m_SwapChainImageViews[i] = create_image_view(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }

}
void AppVulkanImpl::create_render_pass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_SwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = find_depth_format();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;


    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
    m_DeletionQueue.push_function([=]() { vkDestroyRenderPass(m_Device, m_RenderPass, nullptr); }, "RenderPass");
}



void AppVulkanImpl::create_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = create_descriptor_set_layout_binding(0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutBinding sceneLayoutBinding = create_descriptor_set_layout_binding(1, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, sceneLayoutBinding };//, samplerLayoutBinding
    VkDescriptorSetLayoutCreateInfo sceneLayoutInfo = create_layout_info(bindings.data(), bindings.size());
    if (vkCreateDescriptorSetLayout(m_Device, &sceneLayoutInfo, nullptr, &m_SceneSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding objectLayoutBinding = create_descriptor_set_layout_binding(0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    VkDescriptorSetLayoutCreateInfo objectLayoutInfo = create_layout_info(&objectLayoutBinding, 1);
    if (vkCreateDescriptorSetLayout(m_Device, &objectLayoutInfo, nullptr, &m_ObjectSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding textureLayoutBinding = create_descriptor_set_layout_binding(0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkDescriptorSetLayoutCreateInfo textureLayoutInfo = create_layout_info(&textureLayoutBinding, 1);
    if (vkCreateDescriptorSetLayout(m_Device, &textureLayoutInfo, nullptr, &m_TextureSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    m_DeletionQueue.push_function([=]() {
        vkDestroyDescriptorSetLayout(m_Device, m_SceneSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(m_Device, m_ObjectSetLayout, nullptr); 
        vkDestroyDescriptorSetLayout(m_Device, m_TextureSetLayout, nullptr); 
        }, 
       "DescriptorSetLayouts");

}

void AppVulkanImpl::create_graphics_pipeline()
{

    VkDescriptorSetLayout setLayouts[] = { m_SceneSetLayout, m_ObjectSetLayout, m_TextureSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 3; // Optional
    pipelineLayoutInfo.pSetLayouts = setLayouts; // Optional

    if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_TexturePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    m_DeletionQueue.push_function([=]() { vkDestroyPipelineLayout(m_Device, m_TexturePipelineLayout, nullptr); }, "PipelineLAyout");

    m_PipelineBuilder.Device = m_Device;
    m_PipelineBuilder.Pass = m_RenderPass;
    m_PipelineBuilder.Extent = m_SwapChainExtent;
    m_PipelineBuilder.PolygonMode = VK_POLYGON_MODE_FILL;
    m_PipelineBuilder.Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_PipelineBuilder.PipelineLayout = m_TexturePipelineLayout;
    m_PipelineBuilder.VertexShaderName = "TextureShader.vert";
    m_PipelineBuilder.FragmentShaderName = "TextureShader.frag";

    m_TexturePipeline = m_PipelineBuilder.build_pipeline();//;("VikingShader.vert", "VikingShader.frag", m_Device, m_TexturePipelineLayout, m_RenderPass, m_SwapChainExtent, VK_POLYGON_MODE_FILL, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
     
    m_DeletionQueue.push_function([=]() { vkDestroyPipeline(m_Device, m_TexturePipeline, nullptr); }, "Pipeline");

    create_material(m_TexturePipeline, m_TexturePipelineLayout,  "texturematerial");


    VkDescriptorSetLayout setLayouts2[] = { m_SceneSetLayout, m_ObjectSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo2{};
    pipelineLayoutInfo2.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo2.setLayoutCount = 2; // Optional
    pipelineLayoutInfo2.pSetLayouts = setLayouts2; // Optional

    if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo2, nullptr, &m_PlainPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    m_DeletionQueue.push_function([=]() { vkDestroyPipelineLayout(m_Device, m_PlainPipelineLayout, nullptr); }, "PipelineLAyout");


    m_PipelineBuilder.PipelineLayout = m_PlainPipelineLayout;
    m_PipelineBuilder.VertexShaderName = "PlainShader.vert";
    m_PipelineBuilder.FragmentShaderName = "PlainShader.frag";

    m_PlainPipeline = m_PipelineBuilder.build_pipeline();//;("VikingShader.vert", "VikingShader.frag", m_Device, m_TexturePipelineLayout, m_RenderPass, m_SwapChainExtent, VK_POLYGON_MODE_FILL, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    m_DeletionQueue.push_function([=]() { vkDestroyPipeline(m_Device, m_PlainPipeline, nullptr); }, "Pipeline");

    create_material(m_PlainPipeline, m_PlainPipelineLayout, "plainmaterial");

    m_PipelineBuilder.VertexShaderName = "IlluminateShader.vert";
    m_PipelineBuilder.FragmentShaderName = "IlluminateShader.frag";

    m_IlluminatedPipeline = m_PipelineBuilder.build_pipeline();//;("VikingShader.vert", "VikingShader.frag", m_Device, m_TexturePipelineLayout, m_RenderPass, m_SwapChainExtent, VK_POLYGON_MODE_FILL, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    create_material(m_IlluminatedPipeline, m_PlainPipelineLayout, "illuminatematerial");

    m_DeletionQueue.push_function([=]() { vkDestroyPipeline(m_Device, m_IlluminatedPipeline, nullptr); }, "Pipeline");


}

void AppVulkanImpl::create_framebuffers()
{
    m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            m_SwapChainImageViews[i],
            m_DepthImageView
        };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_SwapChainExtent.width;
        framebufferInfo.height = m_SwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }

    }
}

void AppVulkanImpl::create_depth_resources()
{
    VkFormat depthFormat = find_depth_format();
    create_image(m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
    m_DepthImageView = create_image_view(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    //m_DeletionQueue.push_function([=]() {
    //    vkDestroyImage(m_Device, m_DepthImage, nullptr);
    //vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);
    //vkDestroyImageView(m_Device, m_DepthImageView, nullptr); }, "DepthResources");
    transition_image_layout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void AppVulkanImpl::create_texture_image(Texture& texture)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(std::string(Texture::PATH + texture.Filename).c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_Device, stagingBufferMemory);

    stbi_image_free(pixels);


    create_image(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.Image, texture.Memory);

    transition_image_layout(texture.Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(stagingBuffer, texture.Image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transition_image_layout(texture.Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);


    texture.ImageView = create_image_view(texture.Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    m_DeletionQueue.push_function([=]() {
        vkDestroyImage(m_Device, texture.Image, nullptr);
        vkFreeMemory(m_Device, texture.Memory, nullptr);
        vkDestroyImageView(m_Device, texture.ImageView, nullptr);
        }, "Texture");

}



void AppVulkanImpl::create_texture_sampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    m_DeletionQueue.push_function([=]() { vkDestroySampler(m_Device, m_TextureSampler, nullptr); }, "Sampelr");

}

void AppVulkanImpl::load_model()
{


    m_Jet.load_from_obj("fighter_jet.obj", false, true);
    m_Jet.load_animation("spiral.txt");
    upload_mesh(m_Jet);
    m_Meshes.insert(std::make_pair("jet", m_Jet));

    m_Panda.load_from_obj("panda.obj", false);
    m_Panda.load_animation("spiral.txt");
    upload_mesh(m_Panda);    
    m_Meshes.insert(std::make_pair("panda", m_Panda));


    m_Cat.load_from_obj("cat.obj", true);
   // m_Cat.load_animation("spiral.txt");
    upload_mesh(m_Cat);
    m_Meshes.insert(std::make_pair("cat", m_Cat));


    Texture fighterJetMain("BODYMAINCOLORCG.png");
    Texture fighterJetCamo("BODYCAMBUMPCG.png");

    create_texture_image(fighterJetMain);
    create_texture_image(fighterJetCamo);

    m_TextureMap.insert(std::make_pair("fighterJetMain", fighterJetMain));
    m_TextureMap.insert(std::make_pair("fighterJetCamo", fighterJetCamo));

    RenderObject jet;
    jet.MeshHandle = get_mesh("jet");
    jet.MaterialHandle = get_material("texturematerial");
    jet.Model = glm::mat4{ 1.0f };

  //  m_RenderObjects.push_back(jet);

    RenderObject panda;
    panda.MeshHandle = get_mesh("panda");
    panda.MaterialHandle = get_material("plainmaterial");
    panda.Model = glm::mat4{ 1.0f };

    RenderObject cat;
    cat.MeshHandle = get_mesh("cat");
    cat.MaterialHandle = get_material("illuminatematerial");
    cat.Model = glm::mat4{ 1.0f };

    cat.setScale(glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)));

    m_RenderObjects.push_back(cat);
}


void AppVulkanImpl::initialize_buffers()
{

    VkDeviceSize bufferSize = sizeof(CameraBufferObject);

    create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_CameraBuffer, m_CameraBufferMemory);

    vkMapMemory(m_Device, m_CameraBufferMemory, 0, bufferSize, 0, &m_CameraBufferMapped);


    bufferSize = sizeof(SceneData);

    create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Scene.DataBuffer, m_Scene.DataMemory);

    vkMapMemory(m_Device, m_Scene.DataMemory, 0, bufferSize, 0, &m_Scene.DataMapped);

    m_Objects.resize(MAX_FRAMES_IN_FLIGHT);
    bufferSize = sizeof(ObjectData) * 1000;
    for (size_t i = 0; i < m_Objects.size(); i++)
    {
        create_buffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Objects[i].Buffer, m_Objects[i].Memory);
 //       vkMapMemory(m_Device, m_Objects[i].Memory, 0, bufferSize, 0, &m_Objects[i].Mapped);
    }

    m_DeletionQueue.push_function(
        [=]() {
            vkDestroyBuffer(m_Device, m_CameraBuffer, nullptr);
            vkFreeMemory(m_Device, m_CameraBufferMemory, nullptr);
            vkDestroyBuffer(m_Device, m_Scene.DataBuffer, nullptr);
            vkFreeMemory(m_Device, m_Scene.DataMemory, nullptr);
            for (size_t i = 0; i < m_Objects.size(); i++)
            {
                vkDestroyBuffer(m_Device, m_Objects[i].Buffer, nullptr);
                vkFreeMemory(m_Device, m_Objects[i].Memory, nullptr);
            }
        }, "Buffers"
    );
}



void AppVulkanImpl::initialize_descriptors()
{
    std::vector<VkDescriptorPoolSize> sizes =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.maxSets = 10;
    poolInfo.poolSizeCount = (uint32_t)sizes.size();
    poolInfo.pPoolSizes = sizes.data();

    if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    m_DeletionQueue.push_function([=]() { vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr); }, "DescriptorPool");

    std::vector<VkDescriptorSetLayout> sceneLayouts(MAX_FRAMES_IN_FLIGHT, m_SceneSetLayout);
    VkDescriptorSetAllocateInfo sceneLayoutsallocInfo = create_descriptor_alloc_info(sceneLayouts.data(), sceneLayouts.size());


    std::vector<VkDescriptorSetLayout> objectLayouts(MAX_FRAMES_IN_FLIGHT, m_ObjectSetLayout);
    VkDescriptorSetAllocateInfo objectLayoutsallocInfo = create_descriptor_alloc_info(objectLayouts.data(), objectLayouts.size());

    std::vector<VkDescriptorSetLayout> textureLayouts(MAX_FRAMES_IN_FLIGHT, m_TextureSetLayout);
    VkDescriptorSetAllocateInfo textureLayoutsallocInfo = create_descriptor_alloc_info(textureLayouts.data(), textureLayouts.size());

    m_SceneSets.resize(MAX_FRAMES_IN_FLIGHT);
    m_ObjectSets.resize(MAX_FRAMES_IN_FLIGHT);
   // m_TextureSets.resize(MAX_FRAMES_IN_FLIGHT);

    Material* texturedMat = get_material("texturematerial");
    texturedMat->TextureSets.resize(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateDescriptorSets(m_Device, &sceneLayoutsallocInfo, m_SceneSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    if (vkAllocateDescriptorSets(m_Device, &objectLayoutsallocInfo, m_ObjectSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    if (vkAllocateDescriptorSets(m_Device, &textureLayoutsallocInfo, texturedMat->TextureSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }


    const int MAX_OBJECTS = 1000;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo cameraBufferInfo{};
        cameraBufferInfo.buffer = m_CameraBuffer;
        cameraBufferInfo.offset = 0;
        cameraBufferInfo.range = sizeof(CameraBufferObject);

        VkDescriptorBufferInfo sceneBufferInfo{};
        sceneBufferInfo.buffer = m_Scene.DataBuffer;
        sceneBufferInfo.offset = 0;
        sceneBufferInfo.range = sizeof(SceneData);


        VkDescriptorBufferInfo objectBufferInfo;
        objectBufferInfo.buffer = m_Objects[i].Buffer;
        objectBufferInfo.offset = 0;
        objectBufferInfo.range = sizeof(ObjectData) * MAX_OBJECTS;

        VkDescriptorImageInfo imageBufferInfo;
        imageBufferInfo.sampler = m_TextureSampler;
        imageBufferInfo.imageView = m_TextureMap["fighterJetMain"].ImageView;
        imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        //std::array<VkDescriptorBufferInfo, 3> descriptorBufferInfo{ cameraBufferInfo, sceneBufferInfo, objectBufferInfo };

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        descriptorWrites.push_back(write_descriptor_set(m_SceneSets[i], 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cameraBufferInfo));
        descriptorWrites.push_back(write_descriptor_set(m_SceneSets[i], 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, sceneBufferInfo));
        descriptorWrites.push_back(write_descriptor_set(m_ObjectSets[i], 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, objectBufferInfo));
        descriptorWrites.push_back(write_descriptor_image(texturedMat->TextureSets[i], 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageBufferInfo));


        // Now, call vkUpdateDescriptorSets to perform the updates
        vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    //m_Materials["texturematerial"].TextureSets = m_TextureSets;

}

void AppVulkanImpl::initialize_commands()
{
    QueueFamilyIndices queueFamilyIndices = find_queue_families(m_PhysicalDevice);
    {

        //----------------------POOls-------------------------------------------------------------------------------

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
        if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
        m_DeletionQueue.push_function([=]() { vkDestroyCommandPool(m_Device, m_CommandPool, nullptr); }, "CommandPool");


        //-------------------------BUFFERs---------------------------------------------------------------------------

        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());


        if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    //------------------------Upload Context------------------------------------------------------------------------

    VkCommandPoolCreateInfo uploadCommandPoolInfo{};
    uploadCommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    uploadCommandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    uploadCommandPoolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
    //create pool for upload context
    vkCreateCommandPool(m_Device, &uploadCommandPoolInfo, nullptr, &m_UploadContext.CommandPool);

    m_DeletionQueue.push_function([=]() {
        vkDestroyCommandPool(m_Device, m_UploadContext.CommandPool, nullptr);
        }, "UploadCommandPool");

    //allocate the default command buffer that we will use for the instant commands
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_UploadContext.CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    vkAllocateCommandBuffers(m_Device, &allocInfo, &m_UploadContext.CommandBuffer);
}

void AppVulkanImpl::create_sync_objects()
{
    m_SyncObjects.resize(MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_SyncObjects[i].ImageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_SyncObjects[i].RenderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(m_Device, &fenceInfo, nullptr, &m_SyncObjects[i].InFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
        m_DeletionQueue.push_function([=]() { vkDestroySemaphore(m_Device, m_SyncObjects[i].ImageAvailableSemaphore, nullptr); }, "Semaphore1");
        m_DeletionQueue.push_function([=]() { vkDestroySemaphore(m_Device, m_SyncObjects[i].RenderFinishedSemaphore, nullptr); }, "Sempahore2");
        m_DeletionQueue.push_function([=]() { vkDestroyFence(m_Device, m_SyncObjects[i].InFlightFence, nullptr); }, "Fence");
    }
    VkFenceCreateInfo uploadContextFenceCreateInfo{};
    uploadContextFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    vkCreateFence(m_Device, &uploadContextFenceCreateInfo, nullptr, &m_UploadContext.UploadFence);
    m_DeletionQueue.push_function([=]() {
        vkDestroyFence(m_Device, m_UploadContext.UploadFence, nullptr);
        }, "UploadFence");

    //if (vkCreateFence(m_Device, &fenceInfo, nullptr, &m_UploadContext.UploadFence) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create Upload Fence!");
    //}
    //m_DeletionQueue.push_function([=]() { vkDestroyFence(m_Device, m_UploadContext.UploadFence, nullptr); }, "Fence");

}
//
// implementation
VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags /*= 0*/)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

VkSubmitInfo submit_info(VkCommandBuffer* cmd)
{
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = nullptr;

    info.waitSemaphoreCount = 0;
    info.pWaitSemaphores = nullptr;
    info.pWaitDstStageMask = nullptr;
    info.commandBufferCount = 1;
    info.pCommandBuffers = cmd;
    info.signalSemaphoreCount = 0;
    info.pSignalSemaphores = nullptr;

    return info;
}

void AppVulkanImpl::draw_frame(float deltaTime)
{
    if(s_ImGuiEnabled) ImGui::Render();

    static int frameNumber{ 0 };
    vkWaitForFences(m_Device, 1, &m_SyncObjects[m_CurrentFrame].InFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_SyncObjects[m_CurrentFrame].ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        Recreate_swapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    

    vkResetFences(m_Device, 1, &m_SyncObjects[m_CurrentFrame].InFlightFence);
    vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);

    update_camera_buffer();

    float framed = (frameNumber++ / 120.f);
  //  m_Scene.Data.ambientColor = { sin(framed),0,cos(framed),1 };
    m_Scene.Data.ambientColor = { 0.2, 0.2, 0.2, 1.0 };
    m_Scene.Data.sunlightColor = { 1.0, 1.0, 0.2, 1.0 };

    static const float rotationSpeed = static_cast<float>(glm::two_pi<float>()) / 10.0f; // Radians per second for a full rotation in 10 seconds

    float angle = rotationSpeed * deltaTime;
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
    
    m_Scene.Data.sunPosition = rotationMatrix * m_Scene.Data.sunPosition;


    memcpy(m_Scene.DataMapped, &m_Scene.Data, sizeof(SceneData));


  //  record_command_buffer(m_CommandBuffers[m_CurrentFrame], imageIndex);
    draw_objects(m_CommandBuffers[m_CurrentFrame], m_RenderObjects, imageIndex);



    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_SyncObjects[m_CurrentFrame].ImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];
    VkSemaphore signalSemaphores[] = { m_SyncObjects[m_CurrentFrame].RenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_SyncObjects[m_CurrentFrame].InFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = { m_SwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

   result =  vkQueuePresentKHR(m_PresentQueue, &presentInfo);
   if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
       m_FramebufferResized = false;
       Recreate_swapchain();
   }
   else if (result != VK_SUCCESS) {
       throw std::runtime_error("failed to present swap chain image!");
   }


    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
void AppVulkanImpl::init_imgui()
{
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;


    vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &m_ImguiPool);


    // 2: initialize imgui library

    //this initializes the core structures of imgui
    ImGui::CreateContext();

  //  ImGui_ImplVulkan_Init(m_Window);
    ImGui_ImplGlfw_InitForVulkan(m_Window, false);

    //this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_Instance;
    init_info.PhysicalDevice = m_PhysicalDevice;
    init_info.Device = m_Device;
    init_info.Queue = m_GraphicsQueue;
    init_info.DescriptorPool = m_ImguiPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, m_RenderPass);

    //execute a gpu command to upload imgui font textures
    immediate_submit([&](VkCommandBuffer cmd) {
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
        });

    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    //add the destroy the imgui created structures
    m_DeletionQueue.push_function([=]() {

        vkDestroyDescriptorPool(m_Device, m_ImguiPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        }, "ImGui");

}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//--------------------------------HELPER METHODS--------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void AppVulkanImpl::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{
    VkCommandBuffer cmd = m_UploadContext.CommandBuffer;

    //begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
    VkCommandBufferBeginInfo cmdBeginInfo = command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vkBeginCommandBuffer(cmd, &cmdBeginInfo);

    //execute the function
    function(cmd);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit = submit_info(&cmd);


    //submit command buffer to the queue and execute it.
    // _uploadFence will now block until the graphic commands finish execution
    vkQueueSubmit(m_GraphicsQueue, 1, &submit, m_UploadContext.UploadFence);

    vkWaitForFences(m_Device, 1, &m_UploadContext.UploadFence, true, 9999999999);
    vkResetFences(m_Device, 1, &m_UploadContext.UploadFence);

    // reset the command buffers inside the command pool
    vkResetCommandPool(m_Device, m_UploadContext.CommandPool, 0);
}

bool AppVulkanImpl::check_validation_layer_support()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : m_ValidationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    return true;
}

std::vector<const char*> AppVulkanImpl::get_required_extensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ENABLED_VALIDATION_LAYERS) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VkResult AppVulkanImpl::create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pcreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pcreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void AppVulkanImpl::destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void AppVulkanImpl::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

bool AppVulkanImpl::is_device_suitable(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = find_queue_families(device);

    if (!indices.isComplete()) return false;
    if (!check_device_extension_support(device)) return false;

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    if (!supportedFeatures.samplerAnisotropy) return false;



    SwapChainSupportDetails swapChainSupport = query_swap_chain_support(device);
    return !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();

}

QueueFamilyIndices AppVulkanImpl::find_queue_families(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());



    int i = 0;  
    for (const auto& queueFamily : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.GraphicsFamily = i;
        }
        if (presentSupport) {
            indices.PresentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

bool AppVulkanImpl::check_device_extension_support(VkPhysicalDevice device) {


    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails AppVulkanImpl::query_swap_chain_support(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.PresentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR AppVulkanImpl::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR AppVulkanImpl::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D AppVulkanImpl::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;

        //resolution of the window in pixels, not screen coordinates
        glfwGetFramebufferSize(m_Window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkShaderModule AppVulkanImpl::create_shader_module(const std::vector<uint32_t>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
}

void AppVulkanImpl::record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_SwapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    //float flash = abs(sin( imageIndex / 120.f));
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TexturePipeline);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_Panda.VertexBuffer, 0);

    vkCmdBindIndexBuffer(commandBuffer, m_Panda.IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChainExtent.width);
    viewport.height = static_cast<float>(m_SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_SwapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_TexturePipelineLayout, 0, 1, &m_SceneSets[m_CurrentFrame], 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Panda.Indices.size()), 1, 0, 0, 0);
   // vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Panda.Vertices.size()), 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void AppVulkanImpl::Recreate_swapchain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_Window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(m_Device);

    cleanup_swap_chain();
    create_swapchain();
  //  create_image_views();
    create_depth_resources();
    create_framebuffers();
    update_camera_buffer();
}

void AppVulkanImpl::cleanup_swap_chain()
{
    for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(m_Device, m_SwapChainFramebuffers[i], nullptr);
    }

    vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
    vkDestroyImage(m_Device, m_DepthImage, nullptr);
    vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);

    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
        vkDestroyImageView(m_Device, m_SwapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

uint32_t AppVulkanImpl::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties ) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void AppVulkanImpl::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }
   // m_DeletionQueue.push_function([=]() {vkDestroyBuffer(m_Device, buffer, nullptr); } , "");
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
}

void AppVulkanImpl::copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    end_single_time_commands(commandBuffer);

}

void AppVulkanImpl::update_camera_buffer()
{

    CameraBufferObject cbo{};
    cbo.view = glm::lookAt(m_Camera.Position, m_Camera.Position + m_Camera.Front, m_Camera.Up);
    cbo.proj = glm::perspective(glm::radians(m_Camera.Fov), m_SwapChainExtent.width / (float)m_SwapChainExtent.height, 0.1f, 500.0f);

    cbo.proj[1][1] *= -1;

    cbo.pos = glm::vec4(m_Camera.Position,1.0f);

    memcpy(m_CameraBufferMapped, &cbo, sizeof(cbo));
}

void AppVulkanImpl::create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_Device, image, imageMemory, 0);
}

VkCommandBuffer AppVulkanImpl::begin_single_time_commands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void AppVulkanImpl::end_single_time_commands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue);

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void AppVulkanImpl::transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    //barrier.srcAccessMask = 0; // TODO
    //barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (has_stencil_component(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }


    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage /* TODO */, destinationStage /* TODO */,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    end_single_time_commands(commandBuffer);

}

void AppVulkanImpl::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands(commandBuffer);
}

VkImageView AppVulkanImpl::create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

   // m_DeletionQueue.push_function([=]() { vkDestroyImageView(m_Device, imageView, nullptr); }, "ImageView");

    return imageView;
}

VkFormat AppVulkanImpl::find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat AppVulkanImpl::find_depth_format()
{
    return find_supported_format(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void AppVulkanImpl::upload_mesh(Mesh& mesh)
{

    VkDeviceSize bufferSize = sizeof(mesh.Vertices[0]) * mesh.Vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh.Vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_Device, stagingBufferMemory);

    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.VertexBuffer, mesh.VertexBufferMemory);

    copy_buffer(stagingBuffer, mesh.VertexBuffer, bufferSize);

   // vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
   // vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

    m_DeletionQueue.push_function(
        [=]() {
            vkDestroyBuffer(m_Device, mesh.VertexBuffer, nullptr);
            vkFreeMemory(m_Device, mesh.VertexBufferMemory, nullptr);
            vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
            vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        }, "VertexBuffer"
    );

    if (mesh.Indices.size() == 0) return;
    
    bufferSize = sizeof(mesh.Indices[0]) * mesh.Indices.size();

    VkBuffer stagingBuffer2;
    VkDeviceMemory stagingBufferMemory2;
    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer2, stagingBufferMemory2);

   // void* data;
    vkMapMemory(m_Device, stagingBufferMemory2, 0, bufferSize, 0, &data);
    memcpy(data, mesh.Indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_Device, stagingBufferMemory2);

    create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.IndexBuffer, mesh.IndexBufferMemory);

    copy_buffer(stagingBuffer2, mesh.IndexBuffer, bufferSize);

    //vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    //vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

    m_DeletionQueue.push_function(
        [=]() {
            vkDestroyBuffer(m_Device, mesh.IndexBuffer, nullptr);
            vkFreeMemory(m_Device, mesh.IndexBufferMemory, nullptr);
            vkDestroyBuffer(m_Device, stagingBuffer2, nullptr);
            vkFreeMemory(m_Device, stagingBufferMemory2, nullptr);
        }, "IndexBuffer"
    );
}

Material* AppVulkanImpl::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
    return create_material(pipeline, layout, {VK_NULL_HANDLE}, name);
}

Material* AppVulkanImpl::create_material(VkPipeline pipeline, VkPipelineLayout layout, std::vector<VkDescriptorSet> textureSets, const std::string& name)
{
    Material material;
    material.Pipeline = pipeline;
    material.PipelineLayout = layout;
    material.TextureSets = textureSets;
    m_Materials[name] = material;

    return &m_Materials[name];
}

Material* AppVulkanImpl::get_material(std::string name)
{
    return &m_Materials[name];
}

Mesh* AppVulkanImpl::get_mesh(std::string name)
{
    return &m_Meshes[name];
}


void AppVulkanImpl::draw_objects(VkCommandBuffer commandBuffer, std::vector<RenderObject>& renderObjects, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_SwapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    //float flash = abs(sin( imageIndex / 120.f));
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChainExtent.width);
    viewport.height = static_cast<float>(m_SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_SwapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    static auto startTime{ std::chrono::high_resolution_clock::now() };

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


    Mesh* lastMesh = nullptr;
    Material* lastMaterial = nullptr;


    vkMapMemory(m_Device, m_Objects[imageIndex].Memory, 0, sizeof(ObjectData) * renderObjects.size(), 0, &m_Objects[imageIndex].Mapped);


    ObjectData* objectArray = (ObjectData*)m_Objects[imageIndex].Mapped;

    for (size_t i = 0; i < renderObjects.size(); i++)
    {
        if(renderObjects[i].MeshHandle->Animated) renderObjects[i].compute_animation(time);
        objectArray[i].Model = renderObjects[i].Model;
    }

    vkUnmapMemory(m_Device, m_Objects[imageIndex].Memory);

    for (auto& object : renderObjects)
    {
        if (object.MaterialHandle->TextureSets[0] != VK_NULL_HANDLE) {
            //texture descriptor
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.MaterialHandle->PipelineLayout, 2, 1, &object.MaterialHandle->TextureSets[imageIndex], 0, nullptr);

        }
        //only bind the pipeline if it doesn't match with the already bound one
        if (object.MaterialHandle != lastMaterial) {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.MaterialHandle->PipelineLayout, 0, 1, &m_SceneSets[imageIndex], 0, nullptr);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.MaterialHandle->PipelineLayout, 1, 1, &m_ObjectSets[imageIndex], 0, nullptr);
           // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.MaterialHandle->PipelineLayout, 2, 1, &m_TextureSets[imageIndex], 0, nullptr);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.MaterialHandle->Pipeline);
            lastMaterial = object.MaterialHandle;
        }


        //only bind the mesh if it's a different one from last bind
        if (object.MeshHandle != lastMesh) {
            //bind the mesh vertex buffer with offset 0
            VkBuffer vertexBuffers[] = { object.MeshHandle->VertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, object.MeshHandle->IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

            lastMesh = object.MeshHandle;
        }

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object.MeshHandle->Indices.size()), 1, 0, 0, 0);
    }

    if(s_ImGuiEnabled) ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

}

VkDescriptorSetLayoutBinding AppVulkanImpl::create_descriptor_set_layout_binding(int binding, int count, VkDescriptorType type, VkShaderStageFlagBits shaderStageFlag)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = shaderStageFlag;
    layoutBinding.pImmutableSamplers = nullptr; // Optional

    return layoutBinding;
}

VkWriteDescriptorSet AppVulkanImpl::write_descriptor_set(VkDescriptorSet set, int binding, VkDescriptorType type, const VkDescriptorBufferInfo& bufferInfo)
{
    VkWriteDescriptorSet write;
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;// Your destination descriptor set;
    write.dstBinding = binding;// Binding index for the descriptor set;
    write.dstArrayElement = 0;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;
    write.pNext = nullptr;

    return write;
}

VkWriteDescriptorSet AppVulkanImpl::write_descriptor_image(VkDescriptorSet set, int binding, VkDescriptorType type, const VkDescriptorImageInfo& imageInfo)
{
    VkWriteDescriptorSet write;
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;// Your destination descriptor set;
    write.dstBinding = binding;// Binding index for the descriptor set;
    write.dstArrayElement = 0;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = nullptr;
    write.pImageInfo = &imageInfo;
    write.pNext = nullptr;

    return write;
}

VkDescriptorSetLayoutCreateInfo AppVulkanImpl::create_layout_info(VkDescriptorSetLayoutBinding* bindings, size_t size)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(size);
    layoutInfo.pBindings = bindings;

    return layoutInfo;
}

VkDescriptorSetAllocateInfo AppVulkanImpl::create_descriptor_alloc_info(VkDescriptorSetLayout* layouts, size_t size)
{
    VkDescriptorSetAllocateInfo layoutAllocInfo{};
    layoutAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    layoutAllocInfo.descriptorPool = m_DescriptorPool;
    layoutAllocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    layoutAllocInfo.pSetLayouts = layouts;

    return layoutAllocInfo;
}
