#pragma once


#include "VulkanInit.h"

#include "HelperObjects.h"

struct Character {
	glm::ivec2 size;
	glm::ivec2 bearing;
	uint32_t offset;
	uint32_t advance;
};

class Font
{
private:
	std::vector<Vertex> Vertices;
	std::vector<uint32_t> Indices;
	std::unordered_map<char, Character> Characters;
	int Width;
	int Height;
	float invBmpWidth;
	uint32_t bmpHeight;
	VkBuffer buffer;
	VkDeviceMemory memory;

	void create_font();

	static const std::string PATH;
};

