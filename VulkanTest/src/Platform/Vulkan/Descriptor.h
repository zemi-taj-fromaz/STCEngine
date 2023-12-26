#pragma once

#include "VulkanInit.h"

struct BufferWrapper;
class AppVulkanImpl;

struct Descriptor
{
	Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags)
		: descriptorType(descriptorType), shaderFlags(shaderFlags)
	{
	}
	Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags, VkBufferUsageFlagBits usageFlags, size_t bufferSize, std::function<void(AppVulkanImpl*, void*)> updateFunc)
		: Descriptor(descriptorType, shaderFlags, usageFlags, 1, bufferSize, updateFunc, static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
	{
	}

	Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags, VkBufferUsageFlagBits usageFlags, int bufferCount, size_t bufferSize, std::function<void(AppVulkanImpl*, void*)> updateFunc)
		: Descriptor(descriptorType, shaderFlags, usageFlags, bufferCount, bufferSize, updateFunc, static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
	{
	}

	Descriptor(VkDescriptorType descriptorType, VkShaderStageFlagBits shaderFlags, VkBufferUsageFlagBits usageFlags, int bufferCount, size_t bufferSize, std::function<void(AppVulkanImpl*, void*)> updateFunc, VkMemoryPropertyFlagBits propertyFlags);

	bool isDeviceLocal() { return (propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); }

	VkDescriptorType descriptorType;
	VkShaderStageFlagBits shaderFlags;
	VkBufferUsageFlagBits usageFlags;
	std::vector<BufferWrapper> bufferWrappers;
	std::vector<VkDescriptorSet> descriptorSets;
	std::function<void(AppVulkanImpl*, void*)> bufferUpdateFunc;
	VkMemoryPropertyFlagBits propertyFlags;
	Descriptor* tie{ nullptr };
	VkDescriptorSetLayout layout;
};