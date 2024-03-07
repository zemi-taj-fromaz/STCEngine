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
		cbo.proj = glm::perspective(glm::radians(camera.Fov), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 1000.0f);
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

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> wavesUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			WaveData* objectArray = (WaveData*)bufferMapped;

			auto& waves = app->get_waves();

			for (size_t i = 0; i < waves.size(); i++)
			{

				objectArray[i].amplitude = waves[i]->amplitude;
				objectArray[i].direction = waves[i]->direction;
				objectArray[i].origin = waves[i]->origin;
				objectArray[i].frequency = waves[i]->frequency;
				objectArray[i].phase = waves[i]->phase;
				objectArray[i].steepness = waves[i]->steepness;
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

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> globalLightUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			GlobalLight* globalLightObj = (GlobalLight*)bufferMapped;
			auto& globalLight = app->get_global_light();

			if (!globalLight) return true;

			auto& lightProperties = globalLight->get_light_properties();

			globalLightObj->ambientColor = glm::vec4(lightProperties->ambientLight, 1.0f);
			globalLightObj->diffColor = glm::vec4(lightProperties->diffuseLight, 1.0f);
			globalLightObj->specColor = glm::vec4(lightProperties->specularLight, 1.0f);
			globalLightObj->direction = glm::vec4(lightProperties->direction, 1.0f);

			return true;
		};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> heightMapDataUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		HeightMapData* globalLightObj = (HeightMapData*)bufferMapped;
		float time = app->get_delta_time();

		globalLightObj->delta_time = time;
		globalLightObj->ocean_size = 512.0f;
		globalLightObj->resolution = 256.0f;

		return true;
	};
}