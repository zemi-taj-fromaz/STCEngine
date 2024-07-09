#include "BufferUpdateFunctions.h"

#include "BufferUpdateObjects.h"

namespace Functions
{
	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		CameraBufferObject cbo{};
		Camera camera = app->get_camera();
		auto swapChainExtent = app->get_swapchain_extent();
		
		camera.Position = glm::vec3(63.0199f, 12.4609f, 62.8054f);
		//camera.Front = glm::vec3(-0.698872, -0.0636613, -0.712408f);
		//camera.Up = glm::vec3(0.0445815f, 0.997972f, -0.045445f);
		camera.Front = glm::vec3(0.0, 0.0, 1.0);
		camera.Up = glm::vec3(0.0, 1.0, 0.0);



		cbo.view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
		cbo.proj = glm::perspective(glm::radians(camera.Fov), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);

		//std::cout << "Camera Position" << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << std::endl;
		//std::cout << "Camera Front" << camera.Front.x << " " << camera.Front.y << " " << camera.Front.z << std::endl;
		//std::cout << "Camera Up" << camera.Up.x << " " << camera.Up.y << " " << camera.Up.z << std::endl;

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

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> particlesUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{

		auto deltaTime = app->get_delta_time();
		auto time = app->get_total_time();
		auto particlesSize = app->get_particles_size();

		Particle* particles = (Particle*)bufferMapped;

		for (size_t i = 0; i < particlesSize; i++)
		{
			particles[i].position += glm::vec3(particles[i].velocity, 0.0f) * deltaTime;
			particles[i].color = particles[i].color;

			if (particles[i].color.x > 0.8f)
			{
				continue;
			}
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


	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> mousePositionUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			MousePosition ubo{};
			ubo.pos = app->get_mouse_position() - glm::vec2(app->width / 2.0f, app->height / 2.0f);
			ubo.pos = ubo.pos / glm::vec2(app->width / 2.0f, app->height / 2.0f);

			memcpy(bufferMapped, &ubo, sizeof(MousePosition));
			return true;
		};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> disneyShadingParamsUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			DisneyShadingParams ubo = app->get_disney_params();
			
			memcpy(bufferMapped, &ubo, sizeof(DisneyShadingParams));
			return true;
		};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> resolutionUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		Resolution ubo{};
		ubo.resolution = app->get_resolution();

		memcpy(bufferMapped, &ubo, sizeof(Resolution));
		return true;

	};

	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> pointLightsUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{

		PointLight* pointLightsArray = (PointLight*)bufferMapped;
		auto& pointLights = app->get_point_lights();


		for(int i = 0; i < pointLights.size(); i++)
		{
			//pointLights[i]->update(deltaTime, camera.Position);
			auto& lightProperties = pointLights[i]->get_light_properties();

			pointLightsArray[i].position =		glm::vec4(pointLights[i]->get_position(), 1.0f);
			pointLightsArray[i].ambientColor =	glm::vec4(lightProperties->ambientLight, 1.0f);
			pointLightsArray[i].diffColor =		glm::vec4(lightProperties->diffuseLight, 1.0f);
			pointLightsArray[i].specColor =		glm::vec4(lightProperties->specularLight, 1.0f);
			pointLightsArray[i].clq =			glm::vec4(lightProperties->CLQ, 1.0f);

			pointLightsArray[i].size = static_cast<uint32_t>(pointLights.size());
		

		}
		return true;
	};


	std::function<bool(AppVulkanImpl* app, void* bufferMapped)> flashLightsUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
	{
		
		FlashLight* flashLightsArray = (FlashLight*)bufferMapped;
		auto& flashLights = app->get_flash_lights();
		const Camera& camera = app->get_camera();
		//auto deltaTime = app->get_delta_time();

		for (int i = 0; i < flashLights.size(); i++)
		{
		//	flashLights[i]->update(deltaTime, camera.Position);
			auto& lightProperties = flashLights[i]->get_light_properties();

			flashLightsArray[i].position = glm::vec4(flashLights[i]->get_position(), 1.0f);
			flashLightsArray[i].ambientColor = glm::vec4(lightProperties->ambientLight, 1.0f);
			flashLightsArray[i].diffColor = glm::vec4(lightProperties->diffuseLight, 1.0f);
			flashLightsArray[i].specColor = glm::vec4(lightProperties->specularLight, 1.0f);
			flashLightsArray[i].clq = glm::vec4(lightProperties->CLQ, 1.0f);

			flashLightsArray[i].direction = glm::vec4(flashLights[i]->get_direction(), 1.0f);
			flashLightsArray[i].innerCutoff = lightProperties->innerCutoff;
			flashLightsArray[i].outerCutoff = lightProperties->outerCutoff;

			flashLightsArray[i].size = static_cast<uint32_t>(flashLights.size() + 1);
		

			//std::cout << "Position " << flashLightsArray[i].position.x << "," << flashLightsArray[i].position.y << "," << flashLightsArray[i].position.z << std::endl;

		}
		auto& cameraLight = app->get_camera_light();
		int i = flashLights.size();
		if(cameraLight)
		{
			auto& lightProperties = cameraLight->get_light_properties();

			flashLightsArray[i].position = glm::vec4(camera.Position, 1.0f);
			flashLightsArray[i].ambientColor = glm::vec4(lightProperties->ambientLight, 1.0f);
			flashLightsArray[i].diffColor = glm::vec4(lightProperties->diffuseLight, 1.0f);
			flashLightsArray[i].specColor = glm::vec4(lightProperties->specularLight, 1.0f);
			flashLightsArray[i].clq = glm::vec4(lightProperties->CLQ, 1.0f);

			flashLightsArray[i].direction = glm::vec4(camera.direction, 1.0f);
			flashLightsArray[i].innerCutoff = lightProperties->innerCutoff;
			flashLightsArray[i].outerCutoff = lightProperties->outerCutoff;

			flashLightsArray[i].size = static_cast<uint32_t>(flashLights.size() + 1);
		

		}
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


}