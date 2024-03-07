#pragma once

#include "VulkanInit.h"

struct Texture
{
    Texture(int N, std::function<unsigned char* (int width, int height, int channels)> generateTexture) : Width(N), GenerateTexture(generateTexture)
    {}

    Texture(int N) : Width(N)
    {
        GenerateTexture = [](int width, int height, int channels) 
        {
            unsigned char* pixels;
            pixels = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));

            // Generate pixel data with unique colors
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int index = (y * width + x) * channels;

                    pixels[index + 0] = 0;   // Red component
                    pixels[index + 1] = 0;  // Green component
                    pixels[index + 2] = 0;  // Blue component (set to 0 for simplicity)
                    pixels[index + 3] = 255;  // Alpha component (set to full alpha)
                }
            }

            return pixels;
        };
    }

    Texture(std::string filename) : Filename(filename)
    {}

    Texture(const Texture& texture)
    {
        Image = texture.Image;
        ImageView = texture.ImageView;
        Memory = texture.Memory;
        Filename = texture.Filename;
        DescriptorType = texture.DescriptorType;
        Width = texture.Width;
        GenerateTexture = texture.GenerateTexture;
    }

    static const std::string PATH;

    VkImage Image;
    VkImageView ImageView;
    VkDeviceMemory Memory;
    std::string Filename;
    
    VkDescriptorType DescriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };

    int Width;

    std::function<unsigned char* (int width, int height, int channels)> GenerateTexture;
    std::vector<VkDescriptorSet> descriptorSets;

};