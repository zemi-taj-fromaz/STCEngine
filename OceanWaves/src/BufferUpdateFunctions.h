#pragma once

#include <functional>

class AppVulkanImpl;

namespace Functions
{
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> objectsUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> totalTimeUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> resolutionUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> mandelbulbFactorUpdateFunc;
}						 