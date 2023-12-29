#include <Engine.h>
#include <optional>
#include <functional>

class ExampleLayer : public Layer
{
public:
	ExampleLayer() : Layer("Example") 
	{
		std::function<void(AppVulkanImpl* app, void* bufferMapped)> cameraUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			CameraBufferObject cbo{};
			Camera camera = app->get_camera();
			auto swapChainExtent = app->get_swapchain_extent();
			cbo.view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
			cbo.proj = glm::perspective(glm::radians(camera.Fov), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);
			cbo.proj[1][1] *= -1;
			cbo.pos = glm::vec4(camera.Position, 1.0f);
			memcpy(bufferMapped, &cbo, sizeof(cbo));
		};

		std::function<void(AppVulkanImpl* app, void* bufferMapped)> sceneUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
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
		};

		std::function<void(AppVulkanImpl* app, void* bufferMapped)> objectsUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
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
		};

		std::function<void(AppVulkanImpl* app, void* bufferMapped)> particlesUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
		};

		std::function<void(AppVulkanImpl* app, void* bufferMapped)> timeUpdateFunc = [](AppVulkanImpl* app, void* bufferMapped)
		{
			auto deltaTime = app->get_delta_time();
			ParameterUBO ubo{};
			ubo.deltaTime = deltaTime;

			memcpy(bufferMapped, &ubo, sizeof(ParameterUBO));
		};

		auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), cameraUpdateFunc);
		auto scene = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SceneData), sceneUpdateFunc);
		auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, objectsUpdateFunc);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		auto deltaTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), sizeof(float), timeUpdateFunc);
		auto ssboIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Cestica) * 200,
			particlesUpdateFunc,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		auto ssboOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Cestica) * 200,
			particlesUpdateFunc,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		
		ssboIn->tie = ssboOut.get();

		create_descriptors({camera,scene,objects,sampler,deltaTime, ssboIn,ssboOut});

		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;
		TopoloG topology1({ camera,scene,objects,sampler });
		TopoloG topology2({ camera, objects});
		TopoloG topology3({ camera,scene,objects });
		TopoloG topology4({ camera,objects,sampler });
		TopoloG topology5({ camera, sampler });
			  
		TopoloG topologyCompute({ deltaTime, ssboIn, ssboOut });
		//Topoogy topologyEmpty({});

		auto pipelineLayout1 = std::make_shared<PipelineLayout>(topology1);
		auto pipelineLayout2 = std::make_shared<PipelineLayout>(topology2);
		auto pipelineLayout3 = std::make_shared<PipelineLayout>(topology3);
		auto pipelineLayout4 = std::make_shared<PipelineLayout>(topology4);
		auto pipelineLayout5 = std::make_shared<PipelineLayout>(topology5);
		auto pipelineLayout6 = std::make_shared<PipelineLayout>();

		auto pipelineLayoutCompute = std::make_shared<PipelineLayout>(topologyCompute);
		auto pipelineLayoutGraphics = std::make_shared<PipelineLayout>();
		
		create_layouts({pipelineLayout1, pipelineLayout2 , pipelineLayout3 , pipelineLayout4 , pipelineLayout5, pipelineLayoutCompute, pipelineLayoutGraphics});

		std::vector<std::string> textureShaderNames({  "TextureShader.vert", "TextureShader.frag"  });
		std::vector<std::string> plainShaderNames({  "PlainShader.vert", "PlainShader.frag"});
		std::vector<std::string> illuminateShaderNames({  "IlluminateShader.vert", "IlluminateShader.frag"  });
		std::vector<std::string> skyboxShaderNames({  "SkyboxShader.vert", "SkyboxShader.frag"  });
		std::vector<std::string> particleShaderNames({  "ParticleShader.vert", "ParticleShader.frag"  });
		std::vector<std::string> cubemapShaderNames({  "CubemapShader.vert", "CubemapShader.frag"  });
		std::vector<std::string> computeShaderName({  "ComputeShader.comp"  });
		std::vector<std::string> computeShaderNames({  "ComputeShader.vert", "ComputeShader.frag"});

		auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayout1, textureShaderNames);
		auto plainPipeline = std::make_shared<Pipeline>(pipelineLayout3, plainShaderNames);
		auto illuminatePipeline = std::make_shared<Pipeline>(pipelineLayout3, illuminateShaderNames);
		auto skyboxPipeline = std::make_shared<Pipeline>(pipelineLayout5, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto particlesPipeline = std::make_shared<Pipeline>(pipelineLayout4, particleShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		auto cubemapPipeline = std::make_shared<Pipeline>(pipelineLayout4, cubemapShaderNames);
		this->m_ComputePipeline = std::make_shared<Pipeline>(pipelineLayoutCompute, computeShaderName);
		unsigned int DepthTest{ VK_TRUE };
		bool Skybox{ false };
		VkCullModeFlags cullMode{ VK_CULL_MODE_BACK_BIT };
		VkPolygonMode PolygonMode{ VK_POLYGON_MODE_FILL };
		VkPrimitiveTopology Topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
		this->m_ComputeGraphicsPipeline = std::make_shared<Pipeline>(pipelineLayoutGraphics, computeShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		this->m_ComputeGraphicsPipeline->PolygonMode = VK_POLYGON_MODE_POINT;
		this->m_ComputeGraphicsPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		create_pipelines({ texturedPipeline, plainPipeline, illuminatePipeline, skyboxPipeline, particlesPipeline, cubemapPipeline, m_ComputePipeline, m_ComputeGraphicsPipeline});

		std::shared_ptr<Texture> skyboxTex	 = std::make_shared<Texture>("stormydays/");
		std::shared_ptr<Texture> jetTex		 = std::make_shared<Texture>("BODYMAINCOLORCG.png");
		std::shared_ptr<Texture> smokeTex	 = std::make_shared<Texture>("statue.jpg");
		std::shared_ptr<Texture> woodboxTex	 = std::make_shared<Texture>("wood/");
		create_textures({ skyboxTex, jetTex, smokeTex, woodboxTex});

		Mesh jetMesh("fighter_jet.obj");
		Mesh box("skybox.obj");
		Mesh particle("texture - Copy.obj");


		auto jet = std::make_shared<MeshWrapper>(texturedPipeline, jetMesh);
		jet->texture = jetTex;
		jet->animated = "spiral.txt";
		jet->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

		auto skybox = std::make_shared<MeshWrapper>(skyboxPipeline, box);
		skybox->texture = skyboxTex;
		skybox->isSkybox = true;

		auto woodbox = std::make_shared<MeshWrapper>(cubemapPipeline, box);
		woodbox->texture = woodboxTex;

		auto smoke = std::make_shared<MeshWrapper>(particlesPipeline, particle);
		smoke->texture = smokeTex;
		smoke->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		smoke->head = jet;

		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;
		meshWrappers.push_back(jet);
		meshWrappers.push_back(skybox);
		meshWrappers.push_back(woodbox);
		for (int i = 0; i < 150; i++)
		{
			meshWrappers.push_back(smoke);
		}
		create_mesh(meshWrappers);

		//create_particles(
		//	{
		//		//{150, {4, {"texture.obj"}, false, 2}},
		//		{150, {4, {"texture - Copy.obj"}, false, 2}}
		//	}
		//);

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

