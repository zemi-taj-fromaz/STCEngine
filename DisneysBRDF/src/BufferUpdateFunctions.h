#pragma once

#include <functional>
#include <Engine.h>

namespace Functions
{
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> objectsUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> particlesUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> deltaTimeUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> totalTimeUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> resolutionUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> mousePositionUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> disneyShadingParamsUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> pointLightsUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> flashLightsUpdateFunc;
	extern std::function<bool(AppVulkanImpl* app, void* bufferMapped)> globalLightUpdateFunc;
}