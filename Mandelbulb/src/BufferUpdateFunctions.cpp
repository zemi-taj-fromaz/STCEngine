#include "BufferUpdateFunctions.h"
#include <Engine.h>

#include "BufferUpdateObjects.h"



namespace Functions
{
	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		CameraBufferObject cbo{};
		Camera camera = app->get_camera();
		auto swapChainExtent = app->get_swapchain_extent();
		cbo.view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
		cbo.proj = glm::perspective(glm::radians(camera.Fov), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);
		cbo.proj[1][1] *= -1;
		cbo.pos = glm::vec4(camera.Position, 1.0f);
		memcpy(bufferMapped, &cbo, sizeof(cbo));
		return true;
	};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> objectsUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		ObjectData* objectArray = (ObjectData*)bufferMapped;
		auto& renderables = app->get_renderables();
		const Camera& camera = app->get_camera();
		auto deltaTime = app->get_delta_time();
	//	camera.cameraLight->update_light(deltaTime, camera.Position, nullptr);
		auto time = app->get_total_time();

		for (size_t i = 0; i < renderables.size(); i++)
		{
			if (renderables[i]->is_animated())
			{
				renderables[i]->compute_animation(time);
			}
			if (renderables[i]->is_billboard())
			{
				renderables[i]->update_billboard(camera.Position);
			}
			if (renderables[i]->swing())
			{
				renderables[i]->update_swing(time);
			}
			if (renderables[i]->is_attacker())
			{
				if (!renderables[i]->update_attacker(app, camera.Position, deltaTime))
				{
					return false;
				}
			}

		/*	if (renderables[i]->is_light_source())
			{
				renderables[i]->update(deltaTime, camera.Position);
			}*/

			
			renderables[i]->update(deltaTime, camera.Position);

			objectArray[i].Model = renderables[i]->get_model_matrix();
			objectArray[i].Color = renderables[i]->get_color();
		}
		return true;

	};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> deltaTimeUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		auto deltaTime = app->get_delta_time();
		ParameterUBO ubo{};
		ubo.deltaTime = deltaTime;
		memcpy(bufferMapped, &ubo, sizeof(ParameterUBO));
		return true;

	};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> totalTimeUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		auto totalTime = app->get_total_time();
		ParameterUBO ubo{};
		ubo.deltaTime = totalTime;
		memcpy(bufferMapped, &ubo, sizeof(ParameterUBO));
		return true;

	};


	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> resolutionUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		Resolution ubo{};
		ubo.resolution = app->get_resolution();

		memcpy(bufferMapped, &ubo, sizeof(Resolution));
		return true;

	};
	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> mandelbulbFactorUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		int mandelbulbFactor  = app->get_layer()->get_mandelbulb_factor();
		ParameterUBO ubo{};
		ubo.deltaTime =	static_cast<float>(mandelbulbFactor);
		memcpy(bufferMapped, &ubo, sizeof(ParameterUBO));
		return true;

	};


}