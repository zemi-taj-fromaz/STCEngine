#include <optional>
#include <EntryPoint.h>
#include <iostream>

#include "BufferUpdateFunctions.h"
#include "ParticlesCreationFunctions.h"
#include "BufferUpdateObjects.h"

class ExampleLayer : public Layer
{
public:
	ExampleLayer() : Layer("Example")
	{

		//------------------------------ DESCRIPTORS ---------------------------------------------------

		auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);
		//auto scene = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SceneData), Functions::sceneUpdateFunc);
		auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
		auto pointLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(PointLight) * 20, Functions::pointLightsUpdateFunc);
		auto flashLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(FlashLight) * 20, Functions::flashLightsUpdateFunc);
		auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight), Functions::globalLightUpdateFunc);
		auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		//auto albedo = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		//auto normals = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		//auto metallic = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		//auto roughness = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		auto deltaTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT), sizeof(float), Functions::deltaTimeUpdateFunc);
		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		auto reloadTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::reloadTimeUpdateFunc);

		//auto ssboIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
		//	Functions::particlesUpdateFunc);
		//auto ssboOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT), 3, sizeof(Particle) * 10e6,
		//	Functions::particlesUpdateFunc);

	//	ssboIn->tie = ssboOut.get();

	//	ssboOut->particlesCreateFunction = Functions::mandelbulb;

		create_descriptors({ camera,objects, resolution, sampler, totalTime, reloadTime, pointLights, flashLights, globalLight //}, albedo, normals, metallic, roughness
	});// , deltaTime, ssboIn, ssboOut


		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------


		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;
		TopoloG topology1({ camera, objects, sampler });
		TopoloG topology2({ camera, objects });
		TopoloG topology5({ camera, sampler });
		TopoloG topology6({ camera, objects, resolution, totalTime });
		TopoloG topology7({ camera, objects, pointLights, flashLights, globalLight });
		TopoloG topology8({ camera, objects, pointLights, flashLights, globalLight, sampler });

		TopoloG PBRtopology({ camera, objects, pointLights, flashLights, globalLight //}, albedo, normals, metallic, roughness
	});

		//	TopoloG topologyCompute({ deltaTime, ssboIn, ssboOut });

		TopoloG topologyComputeGraphics({ camera });

		TopoloG topologyMT({ objects, resolution });
		TopoloG topologyReload({ objects, resolution, reloadTime });
		//Topoogy topologyEmpty({});

		auto pipelineLayout1 = std::make_shared<PipelineLayout>(topology1);
		auto pipelineLayout2 = std::make_shared<PipelineLayout>(topology2);
		auto pipelineLayout3 = std::make_shared<PipelineLayout>(topology2);
		auto pipelineLayout4 = std::make_shared<PipelineLayout>(topology1);
		auto pipelineLayout5 = std::make_shared<PipelineLayout>(topology5);
		auto pipelineLayout6 = std::make_shared<PipelineLayout>(topology6);
		auto pipelineLayout7 = std::make_shared<PipelineLayout>(topology7);
		auto pipelineLayout8 = std::make_shared<PipelineLayout>(topology8);

		auto pipelineLayoutPBR = std::make_shared<PipelineLayout>(PBRtopology);

		//auto pipelineLayoutCompute = std::make_shared<PipelineLayout>(topologyCompute);
		auto pipelineLayoutGraphics = std::make_shared<PipelineLayout>(topologyComputeGraphics);

		auto layoutMT = std::make_shared<PipelineLayout>(topologyMT);
		auto layoutReload = std::make_shared<PipelineLayout>(topologyReload);

		create_layouts({layoutMT,layoutReload, pipelineLayout1, pipelineLayout2 , pipelineLayout3 , pipelineLayout4 , pipelineLayout5, pipelineLayout6, pipelineLayout7, pipelineLayout8,  pipelineLayoutPBR, pipelineLayoutGraphics });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------

		std::vector<std::string> textureShaderNames({ "TextureShader.vert", "TextureShader.frag" });
		std::vector<std::string> plainShaderNames({ "PlainShader.vert", "PlainShader.frag" });
		std::vector<std::string> illuminateShaderNames({ "IlluminateShader.vert", "IlluminateShader.frag" });
		std::vector<std::string> skyboxShaderNames({ "SkyboxShader.vert", "SkyboxShader.frag" });
		std::vector<std::string> particleShaderNames({ "ParticleShader.vert", "ParticleShader.frag" });
		std::vector<std::string> cubemapShaderNames({ "CubemapShader.vert", "CubemapShader.frag" });
		std::vector<std::string> computeShaderName({ "ComputeShader.comp" });
		std::vector<std::string> computeShaderNames({ "ComputeShader.vert", "ComputeShader.frag" });
		std::vector<std::string> mandelbulbShaderNames({ "MandelbulbShader.vert", "MandelbulbShader.frag" });
		std::vector<std::string> aimShaderNames({ "AimShader.vert", "AimShader.frag" });
		std::vector<std::string> reloadShaderNames({ "ReloadShader.vert", "ReloadShader.frag" });
		//std::vector<std::string> PBRShaderNames({ "PBRShader.vert", "PBRShader.frag" });

		//------------------------------- PIPELINES ----------------------------------------------------------

		auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayout1, textureShaderNames);
		auto plainPipeline = std::make_shared<Pipeline>(pipelineLayout2, plainShaderNames);
		auto illuminatePipeline = std::make_shared<Pipeline>(pipelineLayout8, illuminateShaderNames);
		auto skyboxPipeline = std::make_shared<Pipeline>(pipelineLayout5, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto particlesPipeline = std::make_shared<Pipeline>(pipelineLayout1, particleShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		auto cubemapPipeline = std::make_shared<Pipeline>(pipelineLayout8, cubemapShaderNames);
		auto mandelbulbPipeline = std::make_shared<Pipeline>(pipelineLayout6, mandelbulbShaderNames);

		auto aimPipeline = std::make_shared<Pipeline>(layoutMT, aimShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		aimPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		auto reloadPipeline = std::make_shared<Pipeline>(layoutReload, reloadShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		//aimPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

		auto linePipeline = std::make_shared<Pipeline>(pipelineLayout2, plainShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		linePipeline->Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

	//	auto PBRPipeline = std::make_shared<Pipeline>(pipelineLayoutPBR, PBRShaderNames);

		//	this->m_ComputePipeline = std::make_shared<Pipeline>(pipelineLayoutCompute, computeShaderName);
		//	this->m_ComputeGraphicsPipeline = std::make_shared<Pipeline>(pipelineLayoutGraphics, computeShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
		//	this->m_ComputeGraphicsPipeline->PolygonMode = VK_POLYGON_MODE_POINT;
		//	this->m_ComputeGraphicsPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		create_pipelines({ aimPipeline, texturedPipeline,  reloadPipeline, plainPipeline, illuminatePipeline, skyboxPipeline, particlesPipeline, cubemapPipeline, mandelbulbPipeline// , PBRPipeline
	});//, m_ComputePipeline, m_ComputeGraphicsPipeline});

		//------------------------- TEXTURES ---------------------------------------------------------------

		std::shared_ptr<Texture> deerTex = std::make_shared<Texture>("deerHead.jpeg");
		std::shared_ptr<Texture> skyboxTex = std::make_shared<Texture>("stormydays/");
		std::shared_ptr<Texture> jetTex = std::make_shared<Texture>("BODYMAINCOLORCG.png");
		std::shared_ptr<Texture> smokeTex = std::make_shared<Texture>("statue.jpg");
		std::shared_ptr<Texture> woodboxTex = std::make_shared<Texture>("wood/");
		std::shared_ptr<Texture> grassTex = std::make_shared<Texture>("grass.jpg");
		std::shared_ptr<Texture> retardedTreeTex = std::make_shared<Texture>("retarded_tree.jpg");


		std::shared_ptr<Texture> rustAlbedoTex = std::make_shared<Texture>("rustAlbedo.png");
		std::shared_ptr<Texture> rustNormalsTex = std::make_shared<Texture>("rustNormals.png");
		std::shared_ptr<Texture> rustMetallicTex = std::make_shared<Texture>("rustMetallic.png");
		std::shared_ptr<Texture> rustRoughnessTex = std::make_shared<Texture>("rustRoughness.png");

		create_textures({ skyboxTex, jetTex, smokeTex, deerTex, woodboxTex, grassTex, retardedTreeTex //}, rustAlbedoTex, rustNormalsTex, rustMetallicTex, rustRoughnessTex
	});

		//----------------------- MESH --------------------------------------------------------------------

		Mesh jetMesh("fighter_jet.obj");
		Mesh box("skybox.obj");
		Mesh particle("texture - Copy.obj");
		Mesh catMesh("cat.obj");
		Mesh square("texture - Copy.obj");
		Mesh sphere("sphere.OBJ");
		Mesh treeMesh("low_poly_tree.obj");
		Mesh deerMesh("deer.obj");

		Mesh realSphere(MeshType::Sphere);
		Mesh cube(MeshType::Cube);
		Mesh quadMesh(MeshType::Quad);
		Mesh lineMesh(MeshType::Line);

		Mesh aim1(MeshType::Line);
		Mesh aim2(MeshType::Line);

		Mesh terrain(MeshType::Terrain);

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
		m_Camera.Position = glm::vec3(0.0f, 0.0f, 50.0f);

		auto globalLighter = std::make_shared<LightProperties>(LightType::GlobalLight, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, -1.0f));

		//-------------------- MESH WRAPPERS ----------------------------------------------------------------

		auto skybox = std::make_shared<MeshWrapper>(skyboxPipeline, box);
		skybox->textures.push_back(skyboxTex);
		skybox->isSkybox = true;


		auto aimX = std::make_shared<MeshWrapper>(aimPipeline, aim1);
	//	aimX->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
		aimX->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 1.0f));
		aimX->rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		auto aimY = std::make_shared<MeshWrapper>(aimPipeline, aim2);
		//aimY->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		aimY->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 1.0f));
		aimY->rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		auto line = std::make_shared<MeshWrapper>(aimPipeline, aim2);
		line->rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));


		auto reload = std::make_shared<MeshWrapper>(reloadPipeline, quadMesh);
		reload->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 1.0f));
		reload->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2f, 0.7f, 0.0f));

		auto tree = std::make_shared<MeshWrapper>(texturedPipeline, treeMesh);
		tree->textures.push_back(retardedTreeTex);
		tree->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f));


		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;

		meshWrappers.push_back(skybox);

		meshWrappers.push_back(aimX);
		meshWrappers.push_back(aimY);
		meshWrappers.push_back(reload);

		//meshWrappers.push_back(plane);
	
		std::random_device rd;
		std::mt19937 gen(rd());
		// Generate a random angle in radians
		std::uniform_real_distribution<> angleDistribution(0.0, 360.0f);

		// Generate a random distance within the specified range
		std::uniform_real_distribution<> distanceDistribution(50.0f, 150.0f);

		for (int i = -3; i < 3; i++)
		{
			for (int j = -3; j < 3; j++)
			{
				auto plane = std::make_shared<MeshWrapper>(texturedPipeline, terrain);
				plane->color = glm::vec4(0.2f, 0.2f, 0.8f, 1.0f);
				plane->translation = glm::translate(glm::mat4(1.0f), glm::vec3(i * 800.0f, 0.0f, j*800.0f));
				plane->textures.push_back(grassTex);
				meshWrappers.push_back(plane);

			}
		}
		std::uniform_real_distribution<> treeDistanceDistribution(0.0f, 500.0f);

		for (int i = 0; i < 100; i++)
		{
			double angle = angleDistribution(gen);
			double distance = treeDistanceDistribution(gen);

			double xOffset = distance * cos(angle);
			double zOffset = distance * sin(angle);

			glm::vec3 offset = m_Camera.Position + glm::vec3(xOffset, -5.0f, zOffset);

			auto tree = std::make_shared<MeshWrapper>(texturedPipeline, treeMesh);
			tree->textures.push_back(retardedTreeTex);
			tree->translation = glm::translate(glm::mat4(1.0f), offset);
			tree->scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
			meshWrappers.push_back(tree);
		}

		m_DeerPipeline = texturedPipeline;
		m_DeerTex = deerTex;
		for (int i = 0; i < 10; i++)
		{
			double angle = angleDistribution(gen);
			double distance = distanceDistribution(gen);
			
			double xOffset = distance * cos(angle);
			double zOffset = distance * sin(angle);

			glm::vec3 offset = m_Camera.Position + glm::vec3(xOffset, 0.0f, zOffset);

			auto deer = std::make_shared<MeshWrapper>(m_DeerPipeline, quadMesh);
			//deer->color = glm::vec4(0.4f, 0.06f, 0.0f, 1.0f);
			deer->textures.push_back(m_DeerTex);
			deer->translation = glm::translate(glm::mat4(1.0f), offset);
			deer->scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));

			deer->Attacker = true;

			meshWrappers.push_back(deer);
			//meshWrappers.push_back(smoke);
		}
		create_mesh(meshWrappers);

		//m_Camera = Camera();
		//m_Camera.mesh = jet;

	}

	virtual bool poll_inputs(GLFWwindow* window, float deltaTime) override
	{
		float cameraSpeed = 20.0f;

		static float recall{ 1.0f };

		auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));

		recall += deltaTime;
		recall = std::clamp(recall, 0.0f, 1.0f);

		app->set_reload_time(recall);

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			std::cout << "Scoped in" << std::endl;

			app->set_field_of_view(45.0f);
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			std::cout << "Gun fired" << std::endl;
			if (recall >= 1.0f)
			{
				app->fire_gun();
				recall = 0.0f;
			}

			//app->set_field_of_view(45.0f);
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
		{
			app->set_field_of_view(-45.0f);
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			m_Camera.update_position(cameraSpeed * deltaTime * glm::normalize(glm::vec3(m_Camera.Front.x, 0.0f, m_Camera.Front.z)));
		}


		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			m_Camera.update_position(cameraSpeed * deltaTime * glm::normalize(glm::vec3(m_Camera.Right.x, 0.0f, m_Camera.Right.z)));
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			m_Camera.update_position(-cameraSpeed * deltaTime * glm::normalize(glm::vec3(m_Camera.Front.x, 0.0f, m_Camera.Front.z)));
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			m_Camera.update_position(-cameraSpeed * deltaTime * glm::normalize(glm::vec3(m_Camera.Right.x, 0.0f, m_Camera.Right.z)));
		}


		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			return false;
		}

		return true;
	}

	virtual void timed_action(GLFWwindow* window) override
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());
		// Generate a random angle in radians
		static std::uniform_real_distribution<> angleDistribution(0.0, 360.0f);

		// Generate a random distance within the specified range
		static std::uniform_real_distribution<> distanceDistribution(50.0f, 150.0f);

		static Mesh quadMesh(MeshType::Quad);

		for (int i = 0; i < 3; i++)
		{
			double angle = angleDistribution(gen);
			double distance = distanceDistribution(gen);

			double xOffset = distance * cos(angle);
			double zOffset = distance * sin(angle);

			glm::vec3 offset = m_Camera.Position + glm::vec3(xOffset, 0.0f, zOffset);

			auto deer = std::make_shared<MeshWrapper>(m_DeerPipeline, quadMesh);
//			deer->color = glm::vec4(0.4f, 0.06f, 0.0f, 1.0f);
			deer->textures.push_back(m_DeerTex);
			deer->scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));

			deer->translation = glm::translate(glm::mat4(1.0f), offset);
			deer->Attacker = true;

			auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));

			app->create_mesh(*deer);

			m_Mesh.push_back(deer);
			auto renderObj = std::shared_ptr<RenderObject>(new RenderObject(m_Mesh[m_Mesh.size()-1].get()));
			renderObj->get_mesh()->object = renderObj;
			auto& renderables = app->get_renderables();
			auto& attacker = app->get_attackers();
			renderables.push_back(renderObj);
			if (renderObj->get_mesh()->Attacker) attacker.push_back(renderObj);
			//meshWrappers.push_back(smoke);
		}
	}

private:

	std::shared_ptr<Pipeline> m_DeerPipeline;
	std::shared_ptr<Texture> m_DeerTex;

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

