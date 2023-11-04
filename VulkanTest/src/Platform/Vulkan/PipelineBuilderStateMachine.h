#pragma once

#include "VulkanInit.h"
#include "HelperObjects.h"

#include <shaderc/shaderc.hpp>
#include <filesystem>
#include <fstream>

//TODO - turn pipeline builder into a state machine

class PipelineBuilder
{
public:
	VkPipeline build_pipeline();// (std::string vertShaderName, std::string fragShaderName, VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass pass, VkExtent2D extent, VkPolygonMode polygonMode, VkPrimitiveTopology topology);
	
	std::string VertexShaderName;
	std::string FragmentShaderName;
	VkDevice Device;
	VkPipelineLayout PipelineLayout;
	VkRenderPass Pass; 
	VkExtent2D Extent;
	VkPolygonMode PolygonMode;
	VkPrimitiveTopology Topology;


private:

	const std::string SHADER_PATH = "/resources/shaders/";

	bool compile_shader(std::string sourcePath, shaderc_shader_kind shaderKind, std::vector<uint32_t>& spirvCode);
	VkShaderModule create_shader_module(const std::vector<uint32_t>& code, VkDevice device);
	//std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create(const char* vertShaderName, const char* fragShaderName, VkDevice device);

	//VkPipelineVertexInputStateCreateInfo create_vertex_input_info(std::vector<VkVertexInputAttributeDescription> attributeDescriptions, VkVertexInputBindingDescription bindingDescription);
	//VkPipelineInputAssemblyStateCreateInfo create_input_assembly_info(VkPrimitiveTopology topology);
	//VkPipelineRasterizationStateCreateInfo create_rasterization_info(VkPolygonMode polygonMode);
	//VkPipelineMultisampleStateCreateInfo create_multisample_info();
	//VkPipelineDepthStencilStateCreateInfo create_depth_stencil_info();
	//VkPipelineColorBlendStateCreateInfo create_color_blend_info();
	//VkPipelineDynamicStateCreateInfo create_dynamic_state_info();
	//VkPipelineViewportStateCreateInfo create_viewport_info(VkExtent2D extent);

};
