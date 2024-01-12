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
			auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
			auto pointLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(PointLight) * 20, Functions::pointLightsUpdateFunc);
			auto flashLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(FlashLight) * 20, Functions::flashLightsUpdateFunc);
			auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight) , Functions::globalLightUpdateFunc);
			auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
			auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			//auto albedo = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			//auto normals = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			//auto metallic = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			//auto roughness = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
			auto deltaTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), sizeof(float), Functions::deltaTimeUpdateFunc);
			auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);

			//auto ssboIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
			//	Functions::particlesUpdateFunc);
			//auto ssboOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
			//	Functions::particlesUpdateFunc);
		
		//	ssboIn->tie = ssboOut.get();

		//	ssboOut->particlesCreateFunction = Functions::mandelbulb;

			create_descriptors({ camera,objects, resolution, sampler, totalTime, pointLights, flashLights, globalLight // , albedo, normals, metallic, roughness
	});// , deltaTime, ssboIn, ssboOut


		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------


			using TopoloG = std::vector<std::shared_ptr<Descriptor>>;
			TopoloG topology1({ camera, objects, sampler });
			TopoloG topology2({ camera, objects });
			TopoloG topology5({ camera, sampler });
			TopoloG topology6({ camera, objects, resolution, totalTime });
			TopoloG topology7({ camera, objects, pointLights, flashLights, globalLight });
			TopoloG topology8({ camera, objects, pointLights, flashLights, globalLight, sampler });

			//TopoloG PBRtopology({ camera, objects, pointLights, flashLights, globalLight, albedo, normals, metallic, roughness });
			  
		//	TopoloG topologyCompute({ deltaTime, ssboIn, ssboOut });

			TopoloG topologyComputeGraphics({ camera });
			//Topoogy topologyEmpty({});

			auto pipelineLayout1 = std::make_shared<PipelineLayout>(topology1);
			auto pipelineLayout2 = std::make_shared<PipelineLayout>(topology2);
			auto pipelineLayout3 = std::make_shared<PipelineLayout>(topology2);
			auto pipelineLayout4 = std::make_shared<PipelineLayout>(topology1);
			auto pipelineLayout5 = std::make_shared<PipelineLayout>(topology5);
			auto pipelineLayout6 = std::make_shared<PipelineLayout>(topology6);
			auto pipelineLayout7 = std::make_shared<PipelineLayout>(topology7);
			auto pipelineLayout8 = std::make_shared<PipelineLayout>(topology8);

		//	auto pipelineLayoutPBR = std::make_shared<PipelineLayout>(PBRtopology);


			//auto pipelineLayoutCompute = std::make_shared<PipelineLayout>(topologyCompute);
			auto pipelineLayoutGraphics = std::make_shared<PipelineLayout>(topologyComputeGraphics);
		
			create_layouts({ pipelineLayout1, pipelineLayout2 , pipelineLayout3 , pipelineLayout4 , pipelineLayout5, pipelineLayout6, pipelineLayout7, pipelineLayout8, pipelineLayoutGraphics //}); , pipelineLayoutPBR
	});// , pipelineLayoutCompute, pipelineLayoutGraphics

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
			std::vector<std::string> PBRShaderNames({  "PBRShader.vert", "PBRShader.frag"});

		//------------------------------- PIPELINES ----------------------------------------------------------

			auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayout1, textureShaderNames);
			auto plainPipeline = std::make_shared<Pipeline>(pipelineLayout2, plainShaderNames);
			auto illuminatePipeline = std::make_shared<Pipeline>(pipelineLayout8, illuminateShaderNames);
			auto skyboxPipeline = std::make_shared<Pipeline>(pipelineLayout5, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
			auto particlesPipeline = std::make_shared<Pipeline>(pipelineLayout1, particleShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
			auto cubemapPipeline = std::make_shared<Pipeline>(pipelineLayout8, cubemapShaderNames);
			auto mandelbulbPipeline = std::make_shared<Pipeline>(pipelineLayout6, mandelbulbShaderNames);

			auto linePipeline = std::make_shared<Pipeline>(pipelineLayout2, plainShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
			linePipeline->Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

			//auto PBRPipeline = std::make_shared<Pipeline>(pipelineLayoutPBR, PBRShaderNames);

		//	this->m_ComputePipeline = std::make_shared<Pipeline>(pipelineLayoutCompute, computeShaderName);
			this->m_ComputeGraphicsPipeline = std::make_shared<Pipeline>(pipelineLayoutGraphics, computeShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
			this->m_ComputeGraphicsPipeline->PolygonMode = VK_POLYGON_MODE_POINT;
			this->m_ComputeGraphicsPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			create_pipelines({ texturedPipeline, plainPipeline, illuminatePipeline, skyboxPipeline, particlesPipeline, cubemapPipeline, mandelbulbPipeline, linePipeline // , PBRPipeline
	});//, m_ComputePipeline, m_ComputeGraphicsPipeline});

		//------------------------- TEXTURES ---------------------------------------------------------------

			std::shared_ptr<Texture> skyboxTex	 = std::make_shared<Texture>("stormydays/");
			std::shared_ptr<Texture> jetTex		 = std::make_shared<Texture>("BODYMAINCOLORCG.png");
			std::shared_ptr<Texture> smokeTex	 = std::make_shared<Texture>("statue.jpg");
			std::shared_ptr<Texture> woodboxTex	 = std::make_shared<Texture>("wood/");

			//std::shared_ptr<Texture> rustAlbedoTex	 = std::make_shared<Texture>("rustAlbedo.png");
			//std::shared_ptr<Texture> rustNormalsTex	 = std::make_shared<Texture>("rustNormals.png");
			//std::shared_ptr<Texture> rustMetallicTex	 = std::make_shared<Texture>("rustMetallic.png");
			//std::shared_ptr<Texture> rustRoughnessTex	 = std::make_shared<Texture>("rustRoughness.png");

			create_textures({ skyboxTex, jetTex, smokeTex, woodboxTex // , rustAlbedoTex, rustNormalsTex, rustMetallicTex, rustRoughnessTex
	});

		//----------------------- MESH --------------------------------------------------------------------

			Mesh jetMesh("fighter_jet.obj");
			Mesh box("skybox.obj");
			Mesh particle("texture - Copy.obj");
			Mesh catMesh("cat.obj");
			Mesh square("texture - Copy.obj");
			Mesh sphere("sphere.OBJ");

			Mesh realSphere(MeshType::Sphere);
			Mesh cube(MeshType::Cube);
			Mesh quadMesh(MeshType::Quad);
			Mesh lineMesh(MeshType::Line);

		//-------------------- LIGHTS -----------------------------------------------------------------------

			auto pointLight = std::make_shared<LightProperties>(LightType::PointLight, glm::vec3(0.0f, 1.0f, 0.0f));
			auto flashLight = std::make_shared<LightProperties>(LightType::FlashLight, glm::vec3(1.0f, 0.0f, 0.0f));

			auto cameraFlash = std::make_shared<LightProperties>(LightType::FlashLight, glm::vec3(0.8f, 0.8f, 0.2f));
			cameraFlash->update_light = [this](float time, const glm::vec3& camera_position, Renderable* renderable)
			{
				auto Position = renderable->get_position();
				auto Direction = renderable->get_direction();
				Position = this->m_Camera.Position;
				Direction = this->m_Camera.direction;
				
			};
			cameraFlash->CLQ = glm::vec3(1.0, 0.07, 0.017);

			this->m_Camera = Camera();
			m_Camera.cameraLight = cameraFlash;
			
			auto globalLighter = std::make_shared<LightProperties>(LightType::GlobalLight, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, -1.0f));

		//-------------------- MESH WRAPPERS ----------------------------------------------------------------

		
		auto jet = std::make_shared<MeshWrapper>(texturedPipeline, jetMesh);
		jet->textures.push_back(jetTex);
		jet->animated = "spiral.txt";
		jet->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

		auto skybox = std::make_shared<MeshWrapper>(skyboxPipeline, box);
		skybox->textures.push_back(skyboxTex);
		skybox->isSkybox = true;

		auto woodbox = std::make_shared<MeshWrapper>(cubemapPipeline, box);
		woodbox->textures.push_back(woodboxTex);
		woodbox->illuminated = true;
		woodbox->scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));

		auto smoke = std::make_shared<MeshWrapper>(particlesPipeline, particle);
		smoke->textures.push_back(smokeTex);
		//smoke->scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 20.0f, 20.0f));
		smoke->head = jet;


		//auto quad = std::make_shared<MeshWrapper>(texturedPipeline, quadMesh);
		//quad->textures.push_back(smokeTex);



		//quad->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		

		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;
		//meshWrappers.push_back(sun);
	//	meshWrappers.push_back(moon);
//		meshWrappers.push_back(globe);

		meshWrappers.push_back(jet);
		meshWrappers.push_back(skybox);
	//	meshWrappers.push_back(target);
		meshWrappers.push_back(woodbox);

		//meshWrappers.push_back(cat);
		//meshWrappers.push_back(cubeWrapper);
		////meshWrappers.push_back(quad);
		//meshWrappers.push_back(line);
		//meshWrappers.push_back(line2);
		//meshWrappers.push_back(line3);
		//meshWrappers.push_back(line4);
		//meshWrappers.push_back(line5);
		//meshWrappers.push_back(line6);
	//	meshWrappers.push_back(mandelbulb);
		for (int i = 0; i < 40; i++)
		{
		}
		for (int i = 0; i < 150; i++)
		{
			meshWrappers.push_back(smoke);
		}
		create_mesh(meshWrappers);

		//m_Camera = Camera();
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

