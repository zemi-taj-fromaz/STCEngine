#pragma once
#pragma once

#include "VulkanInit.h"

class Renderable;

enum class LightType
{
	None = 0,
	GlobalLight = 1,
	PointLight = 2,
	FlashLight = 3,
	CameraLight = 4
};

struct LightProperties
{

	LightProperties(LightType type, glm::vec3 diffColor) : lightType(type), diffuseLight(diffColor)
	{
		ambientLight = 0.2f * diffuseLight;
		specularLight = glm::vec3(1.0f);
	}

	LightProperties(LightType type, glm::vec3 diffColor, glm::vec3 direction);

	LightProperties(LightType type, glm::vec3 diffColor, glm::vec3 specColor, glm::vec3 direction) : LightProperties(type, diffColor, specColor)
	{
		this->direction = direction;
	}

	glm::vec3 ambientLight;
	glm::vec3 diffuseLight;
	glm::vec3 specularLight;

	LightType lightType{ LightType::None };

	// https://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation 
	// Distance = 325
	glm::vec3 CLQ{ 1.0, 0.014, 0.0007 };

	glm::vec3 direction;
	float innerCutoff{ 0.91f }; //25 degrees
	float outerCutoff{ 0.82f }; //35 degrees

	std::function<void(float time, const glm::vec3& camera_position, Renderable* renderable)> update_light;

	//	std::function<void(float time, glm::vec3 camera_position, Renderable*)> func = 
};
