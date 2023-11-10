#include "PipelineBuilderStateMachine.h"

VkPipeline PipelineBuilder::build_pipeline()//(std::string vertShaderName, std::string fragShaderName, VkDevice device, VkPipelineLayout pipelineLayout, VkRenderPass pass, VkExtent2D extent, VkPolygonMode polygonMode, VkPrimitiveTopology topology)
{


	auto bindingDescription = Vertex::get_binding_description();
	auto attributeDescriptions = Vertex::get_attribute_descriptions();

	std::vector<uint32_t> vertexShaderSpirv;
	compile_shader(VertexShaderName.c_str(), shaderc_vertex_shader, vertexShaderSpirv);


	std::vector<uint32_t> fragmentShaderSpirv;
	compile_shader(FragmentShaderName.c_str(), shaderc_fragment_shader, fragmentShaderSpirv);

	VkShaderModule vertShaderModule = create_shader_module(vertexShaderSpirv, Device);
	VkShaderModule fragShaderModule = create_shader_module(fragmentShaderSpirv, Device);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";


	auto shaderStages =  std::vector<VkPipelineShaderStageCreateInfo>({ vertShaderStageInfo, fragShaderStageInfo });

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{}; //SEPARATE FUNCTION?
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &Vertex::get_binding_description();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.pNext = nullptr;
	inputAssembly.topology = Topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	rasterizer.polygonMode = PolygonMode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional



	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

		std::vector<VkDynamicState> dynamicStates = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_SCISSOR
		};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(Extent.width);
	viewport.height = static_cast<float>(Extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = Extent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	//auto vertexInputState = create_vertex_input_info(Vertex::get_attribute_descriptions(), Vertex::get_binding_description());
	pipelineInfo.pVertexInputState = &vertexInputInfo; //CreateVertexInputInfo
	pipelineInfo.pInputAssemblyState = &inputAssembly; //FUNC
	pipelineInfo.pViewportState = &viewportState; //FUNC
	pipelineInfo.pRasterizationState = &rasterizer;// (polygonMode); //FUNC
	pipelineInfo.pMultisampleState = &multisampling;// (); //FUNC
	pipelineInfo.pDepthStencilState = &depthStencil;//(); // Optional
	pipelineInfo.pColorBlendState = &colorBlending;//();
	pipelineInfo.pDynamicState = &dynamicState;// ();
	pipelineInfo.layout = PipelineLayout;
	pipelineInfo.renderPass = Pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(Device, fragShaderModule, nullptr);
	vkDestroyShaderModule(Device, vertShaderModule, nullptr);
	return newPipeline;
}

//VkPipelineVertexInputStateCreateInfo PipelineBuilder::create_vertex_input_info(std::vector<VkVertexInputAttributeDescription> attributeDescriptions, VkVertexInputBindingDescription bindingDescription)
//{
//	VkPipelineVertexInputStateCreateInfo vertexInputInfo{}; //SEPARATE FUNCTION?
//	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//	vertexInputInfo.vertexBindingDescriptionCount = 1;
//	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//	return vertexInputInfo;
//}
//
//VkPipelineInputAssemblyStateCreateInfo PipelineBuilder::create_input_assembly_info(VkPrimitiveTopology topology)
//{
//
//	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//	inputAssembly.pNext = nullptr;
//	inputAssembly.topology = topology;
//	inputAssembly.primitiveRestartEnable = VK_FALSE;
//	return inputAssembly;
//}
//
//VkPipelineRasterizationStateCreateInfo PipelineBuilder::create_rasterization_info(VkPolygonMode polygonMode)
//{
//	VkPipelineRasterizationStateCreateInfo rasterizer{};
//	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//	rasterizer.depthClampEnable = VK_FALSE;
//	rasterizer.rasterizerDiscardEnable = VK_FALSE;u
//
//	rasterizer.polygonMode = polygonMode;
//	rasterizer.lineWidth = 1.0f;
//	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//	rasterizer.depthBiasEnable = VK_FALSE;
//	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
//	rasterizer.depthBiasClamp = 0.0f; // Optional
//	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
//	return rasterizer;
//}
//
//
//VkPipelineMultisampleStateCreateInfo PipelineBuilder::create_multisample_info()
//{
//	VkPipelineMultisampleStateCreateInfo multisampling{};
//	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//	multisampling.sampleShadingEnable = VK_FALSE;
//	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//	multisampling.minSampleShading = 1.0f; // Optional
//	multisampling.pSampleMask = nullptr; // Optional
//	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
//	multisampling.alphaToOneEnable = VK_FALSE; // Optional
//	return multisampling;
//}
//
//
//VkPipelineDepthStencilStateCreateInfo PipelineBuilder::create_depth_stencil_info()
//{
//	VkPipelineDepthStencilStateCreateInfo depthStencil{};
//	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//	depthStencil.depthTestEnable = VK_TRUE;
//	depthStencil.depthWriteEnable = VK_TRUE;
//	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
//	depthStencil.depthBoundsTestEnable = VK_FALSE;
//	depthStencil.minDepthBounds = 0.0f; // Optional
//	depthStencil.maxDepthBounds = 1.0f; // Optional
//	depthStencil.stencilTestEnable = VK_FALSE;
//	depthStencil.front = {}; // Optional
//	depthStencil.back = {}; // Optional
//	return depthStencil;
//}
//
//
//VkPipelineColorBlendStateCreateInfo PipelineBuilder::create_color_blend_info()
//{
//
//	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//	colorBlendAttachment.blendEnable = VK_FALSE;
//	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
//	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
//	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
//	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
//	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
//	colorBlendAttachment.blendEnable = VK_TRUE;
//	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
//
//	VkPipelineColorBlendStateCreateInfo colorBlending{};
//	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//	colorBlending.logicOpEnable = VK_FALSE;
//	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
//	colorBlending.attachmentCount = 1;
//	colorBlending.pAttachments = &colorBlendAttachment;
//	colorBlending.blendConstants[0] = 0.0f; // Optional
//	colorBlending.blendConstants[1] = 0.0f; // Optional
//	colorBlending.blendConstants[2] = 0.0f; // Optional
//	colorBlending.blendConstants[3] = 0.0f; // Optional
//
//	return colorBlending;
//}
//
//VkPipelineDynamicStateCreateInfo PipelineBuilder::create_dynamic_state_info()
//{
//	std::vector<VkDynamicState> dynamicStates = {
//	VK_DYNAMIC_STATE_VIEWPORT,
//	VK_DYNAMIC_STATE_SCISSOR
//	};
//
//	VkPipelineDynamicStateCreateInfo dynamicState{};
//	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
//	dynamicState.pDynamicStates = dynamicStates.data();
//
//	return dynamicState;
//}
//
//VkPipelineViewportStateCreateInfo PipelineBuilder::create_viewport_info(VkExtent2D extent)
//{
//	VkViewport viewport{};
//	viewport.x = 0.0f;
//	viewport.y = 0.0f;
//	viewport.width = static_cast<float>(extent.width);
//	viewport.height = static_cast<float>(extent.height);
//	viewport.minDepth = 0.0f;
//	viewport.maxDepth = 1.0f;
//
//	VkRect2D scissor{};
//	scissor.offset = { 0, 0 };
//	scissor.extent = extent;
//
//	VkPipelineViewportStateCreateInfo viewportState{};
//	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//	viewportState.viewportCount = 1;
//	viewportState.pViewports = &viewport;
//	viewportState.scissorCount = 1;
//	viewportState.pScissors = &scissor;
//	return viewportState;
//}
//


bool PipelineBuilder::compile_shader(std::string sourcePath, shaderc_shader_kind shaderKind, std::vector<uint32_t>& spirvCode)
{
	std::filesystem::path currentDir = std::filesystem::current_path();

	// Initialize the shader compiler
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// Load the shader source code from file
	std::ifstream file(currentDir.string() + SHADER_PATH + sourcePath, std::ios::in | std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open the shader file");
	}
	std::vector<char> shaderSource((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	// Compile the shader
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
		shaderSource.data(), shaderSource.size(), shaderKind, sourcePath.c_str(), options);

	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
//		std::cout << result.GetCompilationStatus() << std::endl;
		throw std::runtime_error("Failed to compile shader ! ");
	}

	// Copy the SPIR-V bytecode to the output vector
	spirvCode = { result.begin(), result.end() };
	return true;
}

VkShaderModule PipelineBuilder::create_shader_module(const std::vector<uint32_t>& code, VkDevice device)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(uint32_t);
	createInfo.pCode = code.data();
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}
//
//std::vector<VkPipelineShaderStageCreateInfo> PipelineBuilder::shader_stage_create(const char* vertShaderName, const char* fragShaderName, VkDevice device)
//{
//
//	auto bindingDescription = Vertex::get_binding_description();
//	auto attributeDescriptions = Vertex::get_attribute_descriptions();
//
//	std::vector<uint32_t> vertexShaderSpirv;
//	compile_shader(vertShaderName, shaderc_vertex_shader, vertexShaderSpirv);
//	
//
//	std::vector<uint32_t> fragmentShaderSpirv;
//	compile_shader(fragShaderName, shaderc_fragment_shader, fragmentShaderSpirv);
//
//	VkShaderModule vertShaderModule = create_shader_module(vertexShaderSpirv, device);
//	VkShaderModule fragShaderModule = create_shader_module(fragmentShaderSpirv, device);
//
//	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//	vertShaderStageInfo.module = vertShaderModule;
//	vertShaderStageInfo.pName = "main";
//
//	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//	fragShaderStageInfo.module = fragShaderModule;
//	fragShaderStageInfo.pName = "main";
//
//
//	return std::vector<VkPipelineShaderStageCreateInfo>({ vertShaderStageInfo, fragShaderStageInfo });
//
//}