#include "BufferUpdateFunctions.h"
#include <Engine.h>

#include "BufferUpdateObjects.h"

#define CAMERA_VISION 100000.0f

namespace Functions
{

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		CameraBufferObject cbo{};
		Camera camera = app->get_camera();
		auto swapChainExtent = app->get_swapchain_extent();
		cbo.view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
		cbo.proj = glm::perspective(glm::radians(camera.Fov), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, CAMERA_VISION);
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

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> dispUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			DisplacementData* objectArray = (DisplacementData*)bufferMapped;
			//	camera.cameraLight->update_light(deltaTime, camera.Position, nullptr);
			auto& disp = app->get_displacements();
			auto& norm = app->get_normals();

			for (size_t i = 0; i < disp.size(); i++)
			{
				objectArray[i].Disp = disp[i];
				objectArray[i].Norm = norm[i];
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

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> verticalFlagUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			bool* vertical = (bool*)bufferMapped;
			*vertical = app->get_vertical();
			return true;

		};


	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> resolutionUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		Resolution ubo{};
		ubo.resolution = app->get_resolution();

		memcpy(bufferMapped, &ubo, sizeof(Resolution));
		return true;

	};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> amplitudeUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			Amplitude ubo{};
			ubo.amplitude = app->get_amplitude();
			ubo.lambda = app->get_lambda();

			memcpy(bufferMapped, &ubo, sizeof(Amplitude));
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
		globalLightObj->ocean_size = 256.0f;
		globalLightObj->resolution = 256.0f;

		return true;
	};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> surfaceUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			WaterSurfaceUBO* wateSurface = (WaterSurfaceUBO*)bufferMapped;

			WaterSurfaceUBO& surface = app->get_surface();

			float time = app->get_delta_time();

			wateSurface->camPos = app->get_camera().Position;
			wateSurface->height = surface.height;
			wateSurface->absorpCoef = surface.absorpCoef;
			glm::vec3 s_kWavelengthsRGB_nm{ 680, 550, 440 };
			wateSurface->scatterCoef = surface.scatterCoef;
			wateSurface->backscatterCoef = surface.backscatterCoef;

			wateSurface->terrainColor = surface.terrainColor;

			wateSurface->skyIntensity =  surface.skyIntensity;
			wateSurface->specularIntensity = surface.specularIntensity;
			wateSurface->specularHighlights = surface.specularHighlights;

			wateSurface->sunColor = surface.sunColor;
			wateSurface->sunIntensity =  surface.sunIntensity;

			wateSurface->sunDir = surface.sunDir;
			wateSurface->turbidity = surface.turbidity;
			return true;
		};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> sunPositionUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		SunPositionData* sunPos = (SunPositionData*)bufferMapped;
		auto& sun_pos_data = app->get_sun_pos_data();

		sunPos->Direction = sun_pos_data.Direction;
	//	std::cout << "Sun Direction To GPU " << sunPos->Direction.x << " " << sunPos->Direction.y << " " << sunPos->Direction.z << std::endl;


		return true;
	};
}