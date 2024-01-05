#include <optional>
#include <EntryPoint.h>

#include "BufferUpdateFunctions.h"
#include "ParticlesCreationFunctions.h"
#include "BufferUpdateObjects.h"

class ExampleLayer : public Layer
{
public:
	ExampleLayer() : Layer("Example") 
	{

		//------------------------------ DESCRIPTORS ---------------------------------------------------

			auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);
			//auto scene = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SceneData), Functions::sceneUpdateFunc);
			auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
			auto pointLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(PointLight) * 20, Functions::pointLightsUpdateFunc);
			auto flashLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(FlashLight) * 20, Functions::flashLightsUpdateFunc);
			auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight) , Functions::globalLightUpdateFunc);
			auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
			auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			auto deltaTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), sizeof(float), Functions::deltaTimeUpdateFunc);
			auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		
			//auto ssboIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
			//	Functions::particlesUpdateFunc);
			//auto ssboOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
			//	Functions::particlesUpdateFunc);
		
		//	ssboIn->tie = ssboOut.get();

		//	ssboOut->particlesCreateFunction = Functions::mandelbulb;

			create_descriptors({ camera,objects, resolution, sampler, totalTime, pointLights, flashLights, globalLight });// , deltaTime, ssboIn, ssboOut


		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------


			using TopoloG = std::vector<std::shared_ptr<Descriptor>>;
			TopoloG topology1({ camera, objects, sampler });
			TopoloG topology2({ camera, objects });
			TopoloG topology3({ camera, objects });
			TopoloG topology4({ camera, objects, sampler });
			TopoloG topology5({ camera, sampler });
			TopoloG topology6({ camera, objects, resolution, totalTime });
			TopoloG topology7({ camera, objects, pointLights, flashLights, globalLight });
			  
		//	TopoloG topologyCompute({ deltaTime, ssboIn, ssboOut });

			TopoloG topologyComputeGraphics({ camera });
			//Topoogy topologyEmpty({});

			auto pipelineLayout1 = std::make_shared<PipelineLayout>(topology1);
			auto pipelineLayout2 = std::make_shared<PipelineLayout>(topology2);
			auto pipelineLayout3 = std::make_shared<PipelineLayout>(topology3);
			auto pipelineLayout4 = std::make_shared<PipelineLayout>(topology4);
			auto pipelineLayout5 = std::make_shared<PipelineLayout>(topology5);
			auto pipelineLayout6 = std::make_shared<PipelineLayout>(topology6);
			auto pipelineLayout7 = std::make_shared<PipelineLayout>(topology7);


			//auto pipelineLayoutCompute = std::make_shared<PipelineLayout>(topologyCompute);
			auto pipelineLayoutGraphics = std::make_shared<PipelineLayout>(topologyComputeGraphics);
		
			create_layouts({ pipelineLayout1, pipelineLayout2 , pipelineLayout3 , pipelineLayout4 , pipelineLayout5, pipelineLayout6, pipelineLayout7 });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------


			std::vector<std::string> textureShaderNames({  "TextureShader.vert", "TextureShader.frag"  });
			std::vector<std::string> plainShaderNames({  "PlainShader.vert", "PlainShader.frag"});
			std::vector<std::string> illuminateShaderNames({  "IlluminateShader.vert", "IlluminateShader.frag"  });
			std::vector<std::string> skyboxShaderNames({  "SkyboxShader.vert", "SkyboxShader.frag"  });
			std::vector<std::string> particleShaderNames({  "ParticleShader.vert", "ParticleShader.frag"  });
			std::vector<std::string> cubemapShaderNames({  "CubemapShader.vert", "CubemapShader.frag"  });
			std::vector<std::string> computeShaderName({  "ComputeShader.comp"  });
			std::vector<std::string> computeShaderNames({  "ComputeShader.vert", "ComputeShader.frag"});
			std::vector<std::string> mandelbulbShaderNames({  "MandelbulbShader.vert", "MandelbulbShader.frag"});

		//------------------------------- PIPELINES ----------------------------------------------------------


			auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayout1, textureShaderNames);
			auto plainPipeline = std::make_shared<Pipeline>(pipelineLayout3, plainShaderNames);
			auto illuminatePipeline = std::make_shared<Pipeline>(pipelineLayout7, illuminateShaderNames);
			auto skyboxPipeline = std::make_shared<Pipeline>(pipelineLayout5, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
			auto particlesPipeline = std::make_shared<Pipeline>(pipelineLayout4, particleShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
			auto cubemapPipeline = std::make_shared<Pipeline>(pipelineLayout4, cubemapShaderNames);
			auto mandelbulbPipeline = std::make_shared<Pipeline>(pipelineLayout6, mandelbulbShaderNames);
		//	this->m_ComputePipeline = std::make_shared<Pipeline>(pipelineLayoutCompute, computeShaderName);
		//	this->m_ComputeGraphicsPipeline = std::make_shared<Pipeline>(pipelineLayoutGraphics, computeShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		//	this->m_ComputeGraphicsPipeline->PolygonMode = VK_POLYGON_MODE_POINT;
		//	this->m_ComputeGraphicsPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			create_pipelines({ texturedPipeline, plainPipeline, illuminatePipeline, skyboxPipeline, particlesPipeline, cubemapPipeline, mandelbulbPipeline });//, m_ComputePipeline, m_ComputeGraphicsPipeline});

		//------------------------- TEXTURES ---------------------------------------------------------------

			std::shared_ptr<Texture> skyboxTex	 = std::make_shared<Texture>("stormydays/");
			std::shared_ptr<Texture> jetTex		 = std::make_shared<Texture>("BODYMAINCOLORCG.png");
			std::shared_ptr<Texture> smokeTex	 = std::make_shared<Texture>("statue.jpg");
			std::shared_ptr<Texture> woodboxTex	 = std::make_shared<Texture>("wood/");
			create_textures({ skyboxTex, jetTex, smokeTex, woodboxTex});

		//----------------------- MESH --------------------------------------------------------------------

			Mesh jetMesh("fighter_jet.obj");
			Mesh box("skybox.obj");
			Mesh particle("texture - Copy.obj");
			Mesh catMesh("cat.obj");
			Mesh square("texture - Copy.obj");

		//-------------------- LIGHTS -----------------------------------------------------------------------

			auto pointLight = std::make_shared<LightProperties>(LightType::PointLight, glm::vec3(0.0f, 1.0f, 0.0f));
			auto flashLight = std::make_shared<LightProperties>(LightType::FlashLight, glm::vec3(1.0f, 0.0f, 0.0f));
			auto globalLighter = std::make_shared<LightProperties>(LightType::GlobalLight, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, -1.0f));

		//-------------------- MESH WRAPPERS ----------------------------------------------------------------

		
		auto jet = std::make_shared<MeshWrapper>(texturedPipeline, jetMesh);
		jet->texture = jetTex;
		jet->animated = "spiral.txt";
		jet->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

		auto skybox = std::make_shared<MeshWrapper>(skyboxPipeline, box);
		skybox->texture = skyboxTex;
		skybox->isSkybox = true;

		auto sun = std::make_shared<MeshWrapper>(plainPipeline, box);
		sun->translation = glm::translate(glm::mat4(1.0f), glm::vec3(30.0f, 50.0f, 50.0f));
		sun->scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
		sun->lightProperties = pointLight;
		sun->color = glm::vec4(pointLight->diffuseLight, 1.0f);

		auto moon = std::make_shared<MeshWrapper>(plainPipeline, box);
		moon->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-60.0f, 20.0f, 30.0f));
		moon->scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
		moon->lightProperties = flashLight;
		moon->color = glm::vec4(flashLight->diffuseLight, 1.0f);

		auto globe = std::make_shared<MeshWrapper>(plainPipeline, box);
		globe->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-60.0f, 20.0f, 30.0f));
		globe->scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
		globe->lightProperties = globalLighter;
		globe->color = glm::vec4(globalLighter->diffuseLight, 1.0f);

		auto target = std::make_shared<MeshWrapper>(illuminatePipeline, box);
		target->scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
		target->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		target->illuminated = true;


		auto woodbox = std::make_shared<MeshWrapper>(cubemapPipeline, box);
		woodbox->texture = woodboxTex;
		woodbox->illuminated = true;

		auto smoke = std::make_shared<MeshWrapper>(particlesPipeline, particle);
		smoke->texture = smokeTex;
		smoke->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		smoke->head = jet;

		auto mandelbulb = std::make_shared<MeshWrapper>(mandelbulbPipeline, square);
		mandelbulb->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		mandelbulb->Billboard = true;
		//mandelbulb->translation = glm::translate(glm::mat4(1.0f), glm::vec3(dist(rng), dist(rng), dist(rng)));

		auto cat = std::make_shared<MeshWrapper>(illuminatePipeline, catMesh);
	//	cat->color = glm::vec4(0.0f, 0.5f, 0.0f, 1.0f);
		cat->illuminated = true;
		cat->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));



		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;
		meshWrappers.push_back(sun);
		meshWrappers.push_back(moon);
		meshWrappers.push_back(globe);

		meshWrappers.push_back(jet);
		meshWrappers.push_back(skybox);
	//	meshWrappers.push_back(target);
		//meshWrappers.push_back(woodbox);
		meshWrappers.push_back(cat);
	//	meshWrappers.push_back(mandelbulb);
		for (int i = 0; i < 40; i++)
		{
		}
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

