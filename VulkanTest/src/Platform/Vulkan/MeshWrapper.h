#pragma once

#include "VulkanInit.h"
#include "LightProperties.h"
#include "Mesh.h"

struct Pipeline;
class RenderObject;

struct MeshWrapper
{
	MeshWrapper(std::shared_ptr<Pipeline> pipeline, Mesh mesh) : pipeline(pipeline), mesh(mesh) {}
	MeshWrapper(std::shared_ptr<Pipeline> pipeline, Mesh mesh, bool illuminated) : pipeline(pipeline), mesh(mesh), illuminated(illuminated) {}
	//	MeshWrapper(std::shared_ptr<Pipeline> pipeline, Mesh mesh,  std::shared_ptr<Texture> texture) : pipeline(pipeline), mesh(mesh), texture(texture) {}
	MeshWrapper(std::shared_ptr<Pipeline> pipeline, Mesh mesh, std::string animation) : pipeline(pipeline), mesh(mesh), animated(animation) {}

	void update_position(glm::vec3 position, glm::vec3 Front);

	std::shared_ptr<Pipeline> pipeline;
	Mesh mesh;
	std::vector<std::shared_ptr<Texture>> textures;
	std::optional<std::string> animated{ std::nullopt };
	bool illuminated{ false };
	bool isSkybox{ false };
	bool Billboard{ false };

	std::optional<glm::mat4> translation;
	std::optional<glm::mat4> rotation;
	std::optional<glm::mat4> scale;

	std::shared_ptr<RenderObject> object;
	std::shared_ptr<MeshWrapper> head;
	std::shared_ptr<MeshWrapper> tail;
	std::shared_ptr<LightProperties> lightProperties;

	LightType lightType{ LightType::None };

	bool Swing{ false };
	bool Attacker{ false };
	float SwingRadius;
	glm::vec3 SwingCenter;
	std::vector<float> SwingAngles;

	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
};