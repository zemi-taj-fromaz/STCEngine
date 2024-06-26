#pragma once

#include "Platform/Vulkan/VulkanInit.h"
#include "Platform/Vulkan/Mesh.h"
#include "Platform/Vulkan/Camera.h"
#include "Platform/Vulkan/Descriptor.h"
#include "Platform/Vulkan/LightProperties.h"
#include "Platform/Vulkan/MeshWrapper.h"
//#include "Platform/Vulkan/RenderObject.h"


class AppVulkanImpl;

class RenderObject;
class Renderable;

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

//struct DescriptorSetLayout
//{
//	Descriptor* descriptor;
//	VkDescriptorSetLayout layout;
//};

struct PipelineLayout
{
	PipelineLayout(){}
	PipelineLayout(std::vector<std::shared_ptr<Descriptor>> layouts) : descriptorSetLayout(layouts){}

	std::vector<std::shared_ptr<Descriptor>> descriptorSetLayout;
	VkPipelineLayout layout;


};


struct Pipeline
{
	Pipeline(std::shared_ptr<PipelineLayout> pipelineLayout, std::vector<std::string> ShaderNames) : pipelineLayout(pipelineLayout), ShaderNames(ShaderNames){}
	Pipeline(std::shared_ptr<PipelineLayout> pipelineLayout, std::vector<std::string> ShaderNames, unsigned int DepthTest, bool Skybox, VkCullModeFlags cullMode) 
		: pipelineLayout(pipelineLayout), ShaderNames(ShaderNames), DepthTest(DepthTest), Skybox(Skybox), cullMode(cullMode) {}

	std::shared_ptr<PipelineLayout> pipelineLayout;
	std::vector<std::string> ShaderNames;
	unsigned int DepthTest{ VK_TRUE };
	bool Skybox{ false };
	VkCullModeFlags cullMode{ VK_CULL_MODE_BACK_BIT };
	VkPolygonMode PolygonMode{ VK_POLYGON_MODE_FILL };
	VkPrimitiveTopology Topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
	VkPipeline pipeline;
	std::vector<std::shared_ptr<Texture>> ImageFields;

	void bind(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		for (int i = 0; i < this->pipelineLayout->descriptorSetLayout.size(); ++i)
		{
			if (this->pipelineLayout->descriptorSetLayout[i]->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				|| this->pipelineLayout->descriptorSetLayout[i]->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
				) continue;
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->pipelineLayout->layout, i, 1, &this->pipelineLayout->descriptorSetLayout[i]->descriptorSets[imageIndex], 0, nullptr);
		}

		for (int i = 0; i < this->ImageFields.size(); ++i)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->pipelineLayout->layout, this->pipelineLayout->descriptorSetLayout.size() - this->ImageFields.size() + i, 1, &this->ImageFields[i]->descriptorSets[imageIndex], 0, nullptr);
		}

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, this->pipeline);
	}
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
	virtual ~Layer() 
	{
	};

	virtual void on_update(float timeStep = 0) {}
	virtual bool poll_inputs(GLFWwindow* window, float deltaTime);
	virtual void set_callbacks(GLFWwindow* window);
	virtual void compute_shaders_dispatch(VkCommandBuffer commandBuffer, uint32_t imageIndex, AppVulkanImpl* app){}

	inline const std::string& get_name() const { return m_DebugName; }
	std::vector<std::shared_ptr<Descriptor>>& get_descriptors() { return m_Descriptors; }
	//std::vector<DescriptorSetLayout>& get_layouts() { return m_Layouts; }
	std::vector<std::shared_ptr<PipelineLayout>>& get_pipeline_layouts() { return m_PipelineLayouts; }
	std::vector<std::shared_ptr<Pipeline>>& get_pipelines() { return m_Pipelines; }
	std::vector<std::shared_ptr<Texture>>& get_textures() { return m_Textures; }
	std::vector<std::shared_ptr<Texture>>& get_image_fields() { return m_ImageFields; }
	std::vector<std::shared_ptr<MeshWrapper>>& get_mesh() { return m_Mesh; }
	std::vector<std::shared_ptr<WaveData>>& get_waves() { return m_Waves; }
	std::vector<Particles>& get_particles() { return m_Particles; }
	std::shared_ptr<Pipeline>& get_compute_pipeline() { return m_ComputePipeline; }
	std::shared_ptr<Pipeline>& get_compute_graphics_pipeline() { return m_ComputeGraphicsPipeline; }

	Camera m_Camera;

	bool update_buffers(AppVulkanImpl* app, int imageIndex)
	{
		for (std::shared_ptr<Descriptor>& descriptor : m_Descriptors)
		{
			if (descriptor->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
				descriptor->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
				descriptor->shaderFlags == VK_SHADER_STAGE_COMPUTE_BIT
				)
			{
				continue;
			}
			if(!descriptor->bufferUpdateFunc(app, descriptor->bufferWrappers[imageIndex % descriptor->bufferWrappers.size()].bufferMapped)) return false;
		}
		return true;
	}

	void update_compute_buffers(AppVulkanImpl* app, int imageIndex)
	{
		for (std::shared_ptr<Descriptor>& descriptor : m_Descriptors)
		{
			if (descriptor->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
				descriptor->shaderFlags != VK_SHADER_STAGE_COMPUTE_BIT
				)
			{
				continue;
			}
			if (descriptor->tie) continue;
			descriptor->bufferUpdateFunc(app, descriptor->bufferWrappers[imageIndex % descriptor->bufferWrappers.size()].bufferMapped);
		}
	}

	bool imguiEnabled{ false };

	virtual float get_action_timer() { return 10.0f;  }
	virtual void timed_action(GLFWwindow* window) {}

	void clear_additional() { m_AdditionalMesh.clear(); }
	virtual void draw_imgui(GLFWwindow* window) {};
	virtual int get_mandelbulb_factor() { return 8; }

	virtual bool generate_perlin_noise() { return false;  }

	std::shared_ptr<Pipeline> m_ComputePipeline;
	std::shared_ptr<Pipeline> m_ComputePipeline2;

protected:

	void create_descriptors(std::vector<std::shared_ptr<Descriptor>> descriptors)
	{
		m_Descriptors = descriptors;
	}

	void create_layouts(std::vector<std::shared_ptr<PipelineLayout>> pipelineLayouts)
	{
		m_PipelineLayouts = pipelineLayouts;
	}

	void create_pipelines(std::vector<std::shared_ptr<Pipeline>> pipelines)
	{
		m_Pipelines = pipelines;
	//	m_DeerPipeline = m_Pipelines[1];
	}

	void create_textures(std::vector<std::shared_ptr<Texture>> textures)
	{
		this->m_Textures = textures;
	//	m_DeerTex = m_Textures[3];
	}

	void create_image_fields(std::vector<std::shared_ptr<Texture>> textures)
	{
		this->m_ImageFields = textures;
		for(auto field : m_ImageFields)
		{
			field->DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		}
		
		//	m_DeerTex = m_Textures[3];
	}

	void create_waves(std::vector<std::shared_ptr<WaveData>> waves)
	{
		this->m_Waves = waves;
		//	m_DeerTex = m_Textures[3];
	}

	void create_mesh(std::vector<std::shared_ptr<MeshWrapper>> mesh)
	{
		m_Mesh = mesh;

	}

	void create_particles(std::vector<ParticleCreateStruct> particles)
	{
	}

	std::shared_ptr<Pipeline> m_ComputeGraphicsPipeline;

	std::vector<std::shared_ptr<MeshWrapper>> m_Mesh;
	std::vector<std::shared_ptr<MeshWrapper>> m_AdditionalMesh;
	std::shared_ptr<Pipeline> m_DeerPipeline;
	std::shared_ptr<Texture> m_DeerTex;

private:


	std::string m_DebugName;
	std::vector<std::shared_ptr<Descriptor>> m_Descriptors;
//	std::vector<DescriptorSetLayout> m_Layouts;
	
	std::vector<std::shared_ptr<PipelineLayout>> m_PipelineLayouts;
	std::vector<std::shared_ptr<Pipeline>> m_Pipelines;


	std::vector<Particles> m_Particles;
	std::vector<std::shared_ptr<Texture>> m_Textures;
	std::vector<std::shared_ptr<Texture>> m_ImageFields;
	std::vector<std::shared_ptr<WaveData>> m_Waves;
};

