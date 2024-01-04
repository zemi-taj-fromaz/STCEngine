#include <optional>
#include <EntryPoint.h>

#include "BufferUpdateFunctions.h"
#include "ParticlesCreationFunctions.h"

class ExampleLayer : public Layer
{
public:
	ExampleLayer() : Layer("Example") 
	{

		auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);
		auto scene = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SceneData), Functions::sceneUpdateFunc);
		auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
		auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		auto deltaTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), sizeof(float), Functions::deltaTimeUpdateFunc);
		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		//auto ssboIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
		//	Functions::particlesUpdateFunc);
		//auto ssboOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
		//	Functions::particlesUpdateFunc);
		
	//	ssboIn->tie = ssboOut.get();

	//	ssboOut->particlesCreateFunction = Functions::mandelbulb;

		create_descriptors({ camera,scene,objects, resolution, sampler, totalTime });// , deltaTime, ssboIn, ssboOut


		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;
		TopoloG topology1({ camera,scene,objects,sampler });
		TopoloG topology2({ camera, objects});
		TopoloG topology3({ camera,scene,objects });
		TopoloG topology4({ camera,objects,sampler });
		TopoloG topology5({ camera, sampler });
		TopoloG topology6({ camera, scene, objects, resolution, totalTime });
			  
	//	TopoloG topologyCompute({ deltaTime, ssboIn, ssboOut });

		TopoloG topologyComputeGraphics({ camera });
		//Topoogy topologyEmpty({});

		auto pipelineLayout1 = std::make_shared<PipelineLayout>(topology1);
		auto pipelineLayout2 = std::make_shared<PipelineLayout>(topology2);
		auto pipelineLayout3 = std::make_shared<PipelineLayout>(topology3);
		auto pipelineLayout4 = std::make_shared<PipelineLayout>(topology4);
		auto pipelineLayout5 = std::make_shared<PipelineLayout>(topology5);
		auto pipelineLayout6 = std::make_shared<PipelineLayout>(topology6);


		//auto pipelineLayoutCompute = std::make_shared<PipelineLayout>(topologyCompute);
		auto pipelineLayoutGraphics = std::make_shared<PipelineLayout>(topologyComputeGraphics);
		
		create_layouts({ pipelineLayout1, pipelineLayout2 , pipelineLayout3 , pipelineLayout4 , pipelineLayout5, pipelineLayout6 });// , pipelineLayoutCompute, pipelineLayoutGraphics


		std::vector<std::string> textureShaderNames({  "TextureShader.vert", "TextureShader.frag"  });
		std::vector<std::string> plainShaderNames({  "PlainShader.vert", "PlainShader.frag"});
		std::vector<std::string> illuminateShaderNames({  "IlluminateShader.vert", "IlluminateShader.frag"  });
		std::vector<std::string> skyboxShaderNames({  "SkyboxShader.vert", "SkyboxShader.frag"  });
		std::vector<std::string> particleShaderNames({  "ParticleShader.vert", "ParticleShader.frag"  });
		std::vector<std::string> cubemapShaderNames({  "CubemapShader.vert", "CubemapShader.frag"  });
		std::vector<std::string> computeShaderName({  "ComputeShader.comp"  });
		std::vector<std::string> computeShaderNames({  "ComputeShader.vert", "ComputeShader.frag"});
		std::vector<std::string> mandelbulbShaderNames({  "MandelbulbShader.vert", "MandelbulbShader.frag"});


		auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayout1, textureShaderNames);
		auto plainPipeline = std::make_shared<Pipeline>(pipelineLayout3, plainShaderNames);
		auto illuminatePipeline = std::make_shared<Pipeline>(pipelineLayout3, illuminateShaderNames);
		auto skyboxPipeline = std::make_shared<Pipeline>(pipelineLayout5, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto particlesPipeline = std::make_shared<Pipeline>(pipelineLayout4, particleShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		auto cubemapPipeline = std::make_shared<Pipeline>(pipelineLayout4, cubemapShaderNames);
		auto mandelbulbPipeline = std::make_shared<Pipeline>(pipelineLayout6, mandelbulbShaderNames);
	//	this->m_ComputePipeline = std::make_shared<Pipeline>(pipelineLayoutCompute, computeShaderName);
	//	this->m_ComputeGraphicsPipeline = std::make_shared<Pipeline>(pipelineLayoutGraphics, computeShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
	//	this->m_ComputeGraphicsPipeline->PolygonMode = VK_POLYGON_MODE_POINT;
	//	this->m_ComputeGraphicsPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		create_pipelines({ texturedPipeline, plainPipeline, illuminatePipeline, skyboxPipeline, particlesPipeline, cubemapPipeline, mandelbulbPipeline });//, m_ComputePipeline, m_ComputeGraphicsPipeline});

		std::shared_ptr<Texture> skyboxTex	 = std::make_shared<Texture>("stormydays/");
		std::shared_ptr<Texture> jetTex		 = std::make_shared<Texture>("BODYMAINCOLORCG.png");
		std::shared_ptr<Texture> smokeTex	 = std::make_shared<Texture>("statue.jpg");
		std::shared_ptr<Texture> woodboxTex	 = std::make_shared<Texture>("wood/");
		create_textures({ skyboxTex, jetTex, smokeTex, woodboxTex});

		Mesh jetMesh("fighter_jet.obj");
		Mesh box("skybox.obj");
		Mesh particle("texture - Copy.obj");
		
		Mesh square("texture - Copy.obj");


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

		auto mandelbulb = std::make_shared<MeshWrapper>(mandelbulbPipeline, square);
		mandelbulb->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		mandelbulb->Billboard = true;

		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;
		meshWrappers.push_back(jet);
		meshWrappers.push_back(skybox);
		meshWrappers.push_back(woodbox);
		meshWrappers.push_back(mandelbulb);
		for (int i = 0; i < 150; i++)
		{
			meshWrappers.push_back(smoke);
		}
		create_mesh(meshWrappers);

		m_Camera = Camera();
		//m_Camera.mesh = jet;

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

