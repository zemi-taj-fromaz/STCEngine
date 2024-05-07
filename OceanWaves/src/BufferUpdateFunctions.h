#pragma once

#include <functional>

class AppVulkanImpl;

namespace Functions
{
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> objectsUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> wavesUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> totalTimeUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> resolutionUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> mandelbulbFactorUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> globalLightUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> heightMapDataUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> verticalFlagUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> surfaceUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> amplitudeUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> dispUpdateFunc;// = [](AppVulkanImpl* app, void* bufferMapped)


}						 