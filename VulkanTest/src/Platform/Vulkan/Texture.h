#pragma once

#include "VulkanInit.h"

struct Texture
{
    Texture()
    {}
    Texture(std::string filename) : Filename(filename)
    {}

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
    std::vector<VkDescriptorSet> descriptorSets;

};