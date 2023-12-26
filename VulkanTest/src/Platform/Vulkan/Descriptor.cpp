#include "Descriptor.h"
#include "Layer.h"
#include "AppVulkanImpl.h"

Descriptor::Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags, VkBufferUsageFlagBits usageFlags, int bufferCount, size_t bufferSize, std::function<void(AppVulkanImpl*, void*)> updateFunc, VkMemoryPropertyFlagBits propertyFlags)
	: Descriptor(descriptorType, shaderFlags)
{
	bufferWrappers.resize(bufferCount, { bufferSize });
	this->bufferUpdateFunc = updateFunc;
	this->usageFlags = usageFlags;
	this->propertyFlags = propertyFlags;
}