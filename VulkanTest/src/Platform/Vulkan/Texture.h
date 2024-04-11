#pragma once

#include "VulkanInit.h"

struct Texture
{
    Texture(int N, int M, std::function<float* (int width, int height, int channels)> generateTexture) : Width(N), Height(M),  GenerateTexture(generateTexture)
    {}

    Texture(int N, int M) : Width(N), Height(M)
    {
        GenerateTexture = [](int width, int height, int channels) 
        {
            float* pixels;
            pixels = (float*)malloc(width * height * channels * sizeof(float));

            // Generate pixel data with unique colors
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int index = (y * width + x) * channels;

                    pixels[index + 0] = 0.0f;   // Red component
                    pixels[index + 1] = 0.0f;  // Green component
                    pixels[index + 2] = 0.0f;  // Blue component (set to 0 for simplicity)
                    pixels[index + 3] = 1.0f;  // Alpha component (set to full alpha)
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
        Height = texture.Height;
        GenerateTexture = texture.GenerateTexture;
    }

    static const std::string PATH;

    VkImage Image;
    VkImageView ImageView;
    VkDeviceMemory Memory;
    std::string Filename;
    
    VkDescriptorType DescriptorType{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };

    int Width;
    int Height;

    std::function<float* (int width, int height, int channels)> GenerateTexture;
    std::vector<VkDescriptorSet> descriptorSets;

};