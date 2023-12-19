#pragma once

#include "VulkanInit.h"
#include "HelperObjects.h"

//#include <shaderc/shaderc.hpp>
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
	VkCullModeFlags cullMode{ VK_CULL_MODE_BACK_BIT };
	uint32_t Subpass{ 0 };
	unsigned int DepthTest{ VK_TRUE };
	bool Skybox{ false };


private:

	const std::string SHADER_PATH = "/resources/shaders/";

	bool compile_shader(std::string sourcePath, std::vector<char>& spirvCode);
	VkShaderModule create_shader_module(const std::vector<char>& code, VkDevice device);


};
