#pragma once

#include <functional>
#include <Engine.h>

namespace Functions
{
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> sceneUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> objectsUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> particlesUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> deltaTimeUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> totalTimeUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> resolutionUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> pointLightsUpdateFunc;
	extern std::function<void(AppVulkanImpl* app, void* bufferMapped)> flashLightsUpdateFunc;
}