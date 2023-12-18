#include <Engine.h>
#include <optional>

class ExampleLayer : public Layer
{
public:
	ExampleLayer() : Layer("Example") 
	{
		//VkBufferUsageFlagBits - dodaj u buffer wrapper

		create_descriptors(
			{
				
				{
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject),
					[](AppVulkanImpl* app, void *bufferMapped) 
					{
						CameraBufferObject cbo{};
						Camera camera = app->get_camera();
						auto swapChainExtent = app->get_swapchain_extent();
						cbo.view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
						cbo.proj = glm::perspective(glm::radians(camera.Fov), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);

						cbo.proj[1][1] *= -1;

						cbo.pos = glm::vec4(camera.Position,1.0f);

						memcpy(bufferMapped, &cbo, sizeof(cbo));
					}
				},
				{
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SceneData),
					[](AppVulkanImpl* app, void* bufferMapped)
					{
						static int frameNumber = 0;
						frameNumber++;
						float framed = (frameNumber / 120.f);
						
						auto deltaTime = app->get_delta_time();

						SceneData sceneData{};
						sceneData.ambientColor = { sin(framed),0,cos(framed),1 };
						sceneData.ambientColor = { 0.2, 0.2, 0.2, 1.0 };
						sceneData.sunlightColor = { 1.0, 1.0, 0.2, 1.0 };

						static const float rotationSpeed = static_cast<float>(glm::two_pi<float>()) / 10.0f; // Radians per second for a full rotation in 10 seconds

						float angle = rotationSpeed * deltaTime;
						glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

						sceneData.sunPosition = rotationMatrix * sceneData.sunPosition;
						memcpy(bufferMapped, &sceneData, sizeof(SceneData));
					}
					
				},
				{
					VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000,
					[](AppVulkanImpl* app, void* bufferMapped)
					{

						ObjectData* objectArray = (ObjectData*)bufferMapped;
						auto& renderables = app->get_renderables();
						const Camera& camera = app->get_camera();
						auto deltaTime = app->get_delta_time();
						auto time = app->get_total_time();

						for (size_t i = 0; i < renderables.size(); i++)
						{
							if (renderables[i]->is_animated())
							{
								renderables[i]->compute_animation(time);
							}
							renderables[i]->update(deltaTime, camera.Position);

							objectArray[i].Model = renderables[i]->get_model_matrix();
						}
					}
				} ,
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			}
			);
		
		create_layouts(
			{
				{{0, 1, 2, 3}},
				{{0, 3}},
				{{0, 1, 2}},
				{{0, 2, 3}},
			}
			);

		create_pipelines(
			{
				{0, "TextureShader.vert", "TextureShader.frag"},
				{2, "PlainShader.vert", "PlainShader.frag"},
				{2, "IlluminateShader.vert", "IlluminateShader.frag"},
				{1, "SkyboxShader.vert", "SkyboxShader.frag", VK_FALSE, true, VK_CULL_MODE_FRONT_BIT},
				{3, "ParticleShader.vert", "ParticleShader.frag", VK_TRUE, false, VK_CULL_MODE_NONE },
			}
			);

		create_textures(
			{
				{"stormydays/"},
				{"BODYMAINCOLORCG.png"},
				{"smoke.bmp"},
			}
		);

		create_mesh(
			{
			//	{2, {"cat.obj"}, true, std::nullopt, "spiral.txt"},
				//{1, {"panda.obj"}, false},
				{0, {"fighter_jet.obj"}, false, 1, "spiral.txt"},
				{3, {"skybox.obj"} , false, 0, std::nullopt, true},
			}
		);
		create_particles(
			{
				//{150, {4, {"texture.obj"}, false, 2}},
				{150, {4, {"texture - Copy.obj"}, false, 2}}
			}
		);

	}

private:
	
};


class MyApplication : public Application
{
public:
	MyApplication()
	{
		std::shared_ptr<Layer> layer = std::shared_ptr<Layer>(new ExampleLayer());
		push_layer(layer);
	}
};

std::unique_ptr<Application> CreateApplication()
{
	return std::unique_ptr<Application>(new MyApplication());
}

