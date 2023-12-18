#pragma once

#include "Platform/Vulkan/VulkanInit.h"
#include "Platform/Vulkan/Mesh.h"
#include "Platform/Vulkan/Camera.h"

class AppVulkanImpl;

#include <string>
#include <vector>
#include <functional>

struct BufferWrapper
{
	size_t bufferSize;
	int bufferCount{ 1 };
	VkBuffer buffer;
	VkDeviceMemory deviceMemory;
	void* bufferMapped;
};

struct Descriptor
{
	Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags)
		: descriptorType(descriptorType), shaderFlags(shaderFlags)
	{
	}
	Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags, VkBufferUsageFlagBits usageFlags, size_t bufferSize, std::function<void(AppVulkanImpl*, void*)> updateFunc)
		: Descriptor(descriptorType, shaderFlags, usageFlags, 1, bufferSize, updateFunc)
	{
	}
	Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags, VkBufferUsageFlagBits usageFlags,int bufferCount, size_t bufferSize, std::function<void(AppVulkanImpl*, void*)> updateFunc)
		: Descriptor(descriptorType, shaderFlags)
	{
		bufferWrappers.resize(bufferCount, {bufferSize});
		this->bufferUpdateFunc = updateFunc;
		this->usageFlags = usageFlags;
	}

	VkDescriptorType descriptorType;
	VkShaderStageFlagBits shaderFlags;
	VkBufferUsageFlagBits usageFlags;
	std::vector<BufferWrapper> bufferWrappers;
	std::vector<VkDescriptorSet> descriptorSets;
	std::function<void(AppVulkanImpl*, void*)> bufferUpdateFunc;

};

struct DescriptorSetLayout
{
	Descriptor* descriptor;
	VkDescriptorSetLayout layout;
};

struct PipelineLayout
{
	std::vector<DescriptorSetLayout*> descriptorSetLayouts;
	VkPipelineLayout layout;
};


struct Pipeline
{
	int pipelineLayoutIndex;
	std::string VertexShaderName;
	std::string FragmentShaderName;
	unsigned int DepthTest{ VK_TRUE };
	bool Skybox{ false };
	VkCullModeFlags cullMode{ VK_CULL_MODE_BACK_BIT };
	VkPolygonMode PolygonMode{ VK_POLYGON_MODE_FILL };
	VkPrimitiveTopology Topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
	PipelineLayout* pipelineLayout;
	VkPipeline pipeline;
};

struct MeshWrapper
{
	int PipelineIndex;
	Mesh mesh;
	bool illuminated;
	std::optional<int> textureIndex;
	std::optional<std::string> animated;
	bool isSkybox{ false };
	Pipeline* pipeline;
};


struct ParticleCreateStruct
{
	int particleNumber;
	MeshWrapper meshWrapper;
};

struct Particles
{
	std::vector<MeshWrapper> particlesMesh;
};


class Layer
{
public:
	Layer(const std::string& name = "Layer");
	virtual ~Layer() {};

	virtual void on_update(float timeStep = 0) {}

	inline const std::string& get_name() const { return m_DebugName; }
	std::vector<Descriptor>& get_descriptors() { return m_Descriptors; }
	std::vector<DescriptorSetLayout>& get_layouts() { return m_Layouts; }
	std::vector<PipelineLayout>& get_pipeline_layouts() { return m_PipelineLayouts; }
	std::vector<Pipeline>& get_pipelines() { return m_Pipelines; }
	std::vector<Texture>& get_textures() { return m_Textures; }
	std::vector<MeshWrapper>& get_mesh() { return m_Mesh; }
	std::vector<Particles>& get_particles() { return m_Particles; }

	void update_buffers(AppVulkanImpl* app, int imageIndex)
	{
		for (Descriptor& descriptor : m_Descriptors)
		{
			if (descriptor.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				continue;
			}
			descriptor.bufferUpdateFunc(app, descriptor.bufferWrappers[imageIndex % descriptor.bufferWrappers.size()].bufferMapped);
		}
	}


protected:

	void create_descriptors(std::vector<Descriptor> descriptors)
	{
		m_Descriptors = descriptors;
		m_Layouts.resize(descriptors.size());
		for (int i = 0; i < descriptors.size() ; ++i)
		{
			m_Layouts[i].descriptor = &m_Descriptors[i];
		}
	}

	void create_layouts(std::vector<std::vector<int>> layoutTopology)
	{
		m_PipelineLayouts.resize(layoutTopology.size());
		for (size_t i = 0; i < layoutTopology.size(); i++)
		{
			for (size_t j = 0; j < layoutTopology[i].size(); j++)
			{
				m_PipelineLayouts[i].descriptorSetLayouts.push_back(&m_Layouts[layoutTopology[i][j]]);
			}
		}

	}

	void create_pipelines(std::vector<Pipeline> pipelines)
	{
		m_Pipelines = pipelines;
		for (Pipeline& pipeline : m_Pipelines)
		{
			pipeline.pipelineLayout = &m_PipelineLayouts[pipeline.pipelineLayoutIndex];
		}

	}

	void create_textures(std::vector<Texture> textures)
	{
		this->m_Textures = textures;
	}

	void create_mesh(std::vector<MeshWrapper> mesh)
	{
		m_Mesh = mesh;
		for (MeshWrapper& meshStruct : m_Mesh)
		{
			meshStruct.pipeline = &m_Pipelines[meshStruct.PipelineIndex];
		}
	}

	void create_particles(std::vector<ParticleCreateStruct> particles)
	{
		m_Particles.resize(particles.size());
		for (int i = 0; i < particles.size(); i++)
		{
			particles[i].meshWrapper.pipeline = &m_Pipelines[particles[i].meshWrapper.PipelineIndex];
			m_Particles[i].particlesMesh.resize(particles[i].particleNumber, particles[i].meshWrapper);

		}
	}



private:


	std::string m_DebugName;
	std::vector<Descriptor> m_Descriptors;
	std::vector<DescriptorSetLayout> m_Layouts;
	std::vector<PipelineLayout> m_PipelineLayouts;
	std::vector<Pipeline> m_Pipelines;
	std::vector<MeshWrapper> m_Mesh;
	std::vector<Particles> m_Particles;
	std::vector<Texture> m_Textures;
};

