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

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> dispUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			DisplacementData* objectArray = (DisplacementData*)bufferMapped;
			//	camera.cameraLight->update_light(deltaTime, camera.Position, nullptr);
			auto& disp = app->get_displacements();

			for (size_t i = 0; i < disp.size(); i++)
			{
				objectArray[i].Disp = disp[i];
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
			wateSurface->absorpCoef = surface.absorpCoef;
			glm::vec3 s_kWavelengthsRGB_nm{ 680, 550, 440 };
			wateSurface->scatterCoef = surface.scatterCoef;
			wateSurface->backscatterCoef = surface.backscatterCoef;
			wateSurface->terrainColor = surface.terrainColor;
			wateSurface->skyIntensity =  surface.skyIntensity;
			wateSurface->specularIntensity = surface.specularIntensity;
			wateSurface->specularHighlights = surface.specularHighlights;
			wateSurface->height = surface.height;

			wateSurface->sunColor = surface.sunColor;
			wateSurface->sunIntensity =  surface.sunIntensity ;

			wateSurface->sunDir = surface.sunDir;
			wateSurface->turbidity = surface.turbidity;

			const float t = wateSurface->turbidity;

			wateSurface->A = glm::vec3(0.1787 * wateSurface->turbidity - 1.4630,
				-0.0193 * wateSurface->turbidity - 0.2592,
				-0.0167 * wateSurface->turbidity - 0.2608);
			wateSurface->B = glm::vec3(-0.3554 * wateSurface->turbidity + 0.4275,
				-0.0665 * wateSurface->turbidity + 0.0008,
				-0.0950 * wateSurface->turbidity + 0.0092);
			wateSurface->C = glm::vec3(-0.0227 * wateSurface->turbidity + 5.3251,
				-0.0004 * wateSurface->turbidity + 0.2125,
				-0.0079 * wateSurface->turbidity + 0.2102);
			wateSurface->D = glm::vec3(0.1206 * wateSurface->turbidity - 2.5771,
				-0.0641 * wateSurface->turbidity - 0.8989,
				-0.0441 * wateSurface->turbidity - 1.6537);
			wateSurface->E = glm::vec3(-0.0670 * wateSurface->turbidity + 0.3703,
				-0.0033 * wateSurface->turbidity + 0.0452,
				-0.0109 * wateSurface->turbidity + 0.0529);

			float thetaSun = glm::acos( glm::max(glm::dot(wateSurface->sunDir, glm::vec3(0.0f, 1.0f, 0.0f)), 0.0f));

			const float chi = (4.0 / 9.0 - t / 120.0) * (M_PI - 2.0 * thetaSun);
			const float Yz = (4.0453 * t - 4.9710) * glm::tan(chi) - 0.2155 * t + 2.4192;

			const float theta2 = thetaSun * thetaSun;
			const float theta3 = theta2 * thetaSun;
			const float t2 = t * t;

			const float xz =
				(0.00165 * theta3 - 0.00375 * theta2 + 0.00209 * thetaSun) * t2 +
				(-0.02903 * theta3 + 0.06377 * theta2 - 0.03202 * thetaSun + 0.00394) * t +
				(0.11693 * theta3 - 0.21196 * theta2 + 0.06052 * thetaSun + 0.25886);

			const float yz =
				(0.00275 * theta3 - 0.00610 * theta2 + 0.00317 * thetaSun + 0.0) * t2 +
				(-0.04214 * theta3 + 0.08970 * theta2 - 0.04153 * thetaSun + 0.00516) * t +
				(0.15346 * theta3 - 0.26756 * theta2 + 0.06670 * thetaSun + 0.26688);

			float gamma = thetaSun;
			float cosGamma = glm::cos(gamma);

			wateSurface->ZenithLum = glm::vec3(Yz, xz, yz);
			wateSurface->ZeroThetaSun = (
				1.0f + wateSurface->A * glm::exp(wateSurface->B / glm::cos(0.0f))
				) * (
					1.0f + wateSurface->C * glm::exp(wateSurface->D* gamma) +
					wateSurface->E * cosGamma * cosGamma
					);

			return true;
		};
}