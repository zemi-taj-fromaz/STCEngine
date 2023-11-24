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

struct Mesh
{
    Mesh()
    {

    }

    Mesh(const Mesh& mesh)
    {
        Vertices = mesh.Vertices;
        Indices = mesh.Indices;
        VertexBuffer = mesh.VertexBuffer;
        IndexBuffer = mesh.IndexBuffer;
        VertexBufferMemory = mesh.VertexBufferMemory;
        IndexBufferMemory = mesh.IndexBufferMemory;
        Animation = mesh.Animation;
        Animated = mesh.Animated;
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


    bool load_from_obj(std::string filename, bool illuminated, bool texture = false);
    bool load_animation(std::string filename);
};

struct Material
{
    std::vector<VkDescriptorSet> TextureSets; //texture defaulted to null
    VkPipeline Pipeline;
    VkPipelineLayout PipelineLayout;
};

struct RenderObject
{
    Mesh* MeshHandle;
    Material* MaterialHandle;

    bool isSkybox{ false };

    glm::mat4 Model{ glm::mat4(1.0f) };

    void setTranslation(glm::mat4& translation)
    {
        Translation = translation;
        compute_model_matrix();
    }
    void setRotation(glm::mat4& rotation)
    {
        Rotation = rotation;
        compute_model_matrix();
    }
    void setScale(glm::mat4& scale)
    {
        Scale = scale;
        compute_model_matrix();
    }



    void compute_animation(float time)
    {

        int segment = static_cast<int>(std::floor(time));

        time -= std::floor(time);
        
        auto animation = this->MeshHandle->Animation;

        glm::mat4x3 R = {
             {animation[segment % animation.size()]},
             {animation[(segment + 1) % animation.size()]},
             {animation[(segment + 2) % animation.size()]},
             {animation[(segment + 3) % animation.size()]}
        };
        glm::vec4 T = glm::vec4(glm::pow(time, 3.0f), glm::pow(time, 2.0f), time, 1.0f);

        setTranslation(glm::translate(glm::mat4(1.0f),glm::vec3(R * this->MeshHandle->B * T / 6.0f)));

        glm::vec3 T_d = glm::vec3(glm::pow(time, 2.0f), time, 1.0f);

        glm::vec3 GoalRotation = glm::vec3(glm::mat3(R * this->MeshHandle->B_d) * T_d ) / 2.0f;
        glm::vec3 InitialRotation = glm::vec3(0.0f, 1.0f, 0.0f);

        glm::vec3 rotationAxis = glm::normalize(glm::cross(InitialRotation, GoalRotation));
        float rotationAngle = std::acos(glm::dot(glm::normalize(InitialRotation), glm::normalize(GoalRotation))) * 180.0f / static_cast<float>(M_PI);

        //setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
        setRotation(glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), rotationAxis));
    }
private:

    glm::mat4 Translation{ glm::mat4(1.0f) };
    glm::mat4 Rotation{ glm::mat4(1.0f) };
    glm::mat4 Scale{ glm::mat4(1.0f) };

    void compute_model_matrix()
    {
        Model = Translation * Rotation * Scale;
    }
};

struct Camera
{
    Camera()
    {
       // std::cout << direction.x << " " << direction.y << direction.z << std::endl;
    }

    glm::vec3 Position{ glm::vec3(0.0f, 0.0f, 100.0f) };

    float Yaw = -90.0f;
    float Pitch = 0.0f;
    float Fov = 45.0f;

    glm::vec3 direction{
        cos(glm::radians(Yaw))* cos(glm::radians(Pitch)),
        sin(glm::radians(Pitch)),
        sin(glm::radians(Yaw))* cos(glm::radians(Pitch))
    };

    
    glm::vec3 Front{ glm::normalize(direction)};
  //  glm::vec3 Front{ 0.0f,0.0f,-1.0f};
    glm::vec3 Right{ glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), Front)) };
    glm::vec3 Up{ glm::cross(Front, Right) };



    void process_mouse_movement(float xoffset, float yoffset)
    {
        Yaw += xoffset;
        Pitch += yoffset;

        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;

        direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        direction.y = sin(glm::radians(Pitch));
        direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(direction);
        Right = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), Front));
        Up = glm::cross(Front, Right);
    }

    void set_field_of_view(float yoffset)
    {
        Fov -= yoffset;
        if (Fov < 1.0f)
            Fov = 1.0f;
        if (Fov > 45.0f)
            Fov = 45.0f;
    }
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

struct Texture
{
    Texture()
    {}
    Texture(std::string filename) : Filename(filename)
    {

    }

    Texture(const Texture& texture)
    {
        Image = texture.Image;
        ImageView = texture.ImageView;
        Memory = texture.Memory;
        Filename = texture.Filename;
    }

    static const std::string PATH;

    VkImage Image;
    VkImageView ImageView;
    VkDeviceMemory Memory;
    std::string Filename;
};




