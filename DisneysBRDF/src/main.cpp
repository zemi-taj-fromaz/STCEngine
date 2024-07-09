#include <optional>
#include <EntryPoint.h>

#include "BufferUpdateFunctions.h"
#include "ParticlesCreationFunctions.h"
#include "BufferUpdateObjects.h"

#include "../../../VulkanTest/vendor/imgui/imgui.h"
#include "../../../VulkanTest/vendor/imgui/imgui_impl_vulkan.h"
#include "../../../VulkanTest/vendor/imgui/imgui_impl_glfw.h"

class ExampleLayer : public Layer
{
public:
	ExampleLayer() : Layer("Example") 
	{
		imguiEnabled = true;
	

		//------------------------------ DESCRIPTORS ---------------------------------------------------

			auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);
			auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
			auto pointLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(PointLight) * 20, Functions::pointLightsUpdateFunc);
			auto flashLights = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(FlashLight) * 20, Functions::flashLightsUpdateFunc);
			auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight) , Functions::globalLightUpdateFunc);
			auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
			auto mousePos = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(MousePosition), Functions::mousePositionUpdateFunc);
			auto disneyShadingParams = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(DisneyShadingParams), Functions::disneyShadingParamsUpdateFunc);
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

			create_descriptors({ camera,objects, resolution, disneyShadingParams, mousePos, sampler, totalTime, pointLights, flashLights, globalLight // , albedo, normals, metallic, roughness
	});// , deltaTime, ssboIn, ssboOut


		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------


			using TopoloG = std::vector<std::shared_ptr<Descriptor>>;
			TopoloG topology1({ camera, objects, sampler });
			TopoloG topology2({ camera, objects });
			TopoloG topologyDisney({ camera, objects, mousePos, disneyShadingParams });
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
			auto pipelineLayoutDisney = std::make_shared<PipelineLayout>(topologyDisney);
			auto pipelineLayout3 = std::make_shared<PipelineLayout>(topology2);
			auto pipelineLayout4 = std::make_shared<PipelineLayout>(topology1);
			auto pipelineLayout5 = std::make_shared<PipelineLayout>(topology5);
			auto pipelineLayout6 = std::make_shared<PipelineLayout>(topology6);
			auto pipelineLayout7 = std::make_shared<PipelineLayout>(topology7);
			auto pipelineLayout8 = std::make_shared<PipelineLayout>(topology8);

		//	auto pipelineLayoutPBR = std::make_shared<PipelineLayout>(PBRtopology);


			//auto pipelineLayoutCompute = std::make_shared<PipelineLayout>(topologyCompute);
			auto pipelineLayoutGraphics = std::make_shared<PipelineLayout>(topologyComputeGraphics);
		
			create_layouts({ pipelineLayout1, pipelineLayout2 ,pipelineLayoutDisney, pipelineLayout3 , pipelineLayout4 , pipelineLayout5, pipelineLayout6, pipelineLayout7, pipelineLayout8, pipelineLayoutGraphics //}); , pipelineLayoutPBR
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
			std::vector<std::string> disneyShaderNames({  "DisneyShader.vert", "DisneyShader.frag"});
			std::vector<std::string> mandelbulbShaderNames({  "MandelbulbShader.vert", "MandelbulbShader.frag"});
			std::vector<std::string> PBRShaderNames({  "PBRShader.vert", "PBRShader.frag"});

		//------------------------------- PIPELINES ----------------------------------------------------------

			auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayout1, textureShaderNames);
			auto plainPipeline = std::make_shared<Pipeline>(pipelineLayout2, plainShaderNames);
			auto disneyPipeline = std::make_shared<Pipeline>(pipelineLayoutDisney, disneyShaderNames);
			auto illuminatePipeline = std::make_shared<Pipeline>(pipelineLayout8, illuminateShaderNames);
			auto skyboxPipeline = std::make_shared<Pipeline>(pipelineLayout5, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
			auto particlesPipeline = std::make_shared<Pipeline>(pipelineLayout1, particleShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
			auto cubemapPipeline = std::make_shared<Pipeline>(pipelineLayout8, cubemapShaderNames);
			auto mandelbulbPipeline = std::make_shared<Pipeline>(pipelineLayout6, mandelbulbShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);

			auto linePipeline = std::make_shared<Pipeline>(pipelineLayout2, plainShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
			linePipeline->Topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

			//auto PBRPipeline = std::make_shared<Pipeline>(pipelineLayoutPBR, PBRShaderNames);

		//	this->m_ComputePipeline = std::make_shared<Pipeline>(pipelineLayoutCompute, computeShaderName);
			this->m_ComputeGraphicsPipeline = std::make_shared<Pipeline>(pipelineLayoutGraphics, computeShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);
			this->m_ComputeGraphicsPipeline->PolygonMode = VK_POLYGON_MODE_POINT;
			this->m_ComputeGraphicsPipeline->Topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			create_pipelines({ texturedPipeline, skyboxPipeline, particlesPipeline, plainPipeline, disneyPipeline // , PBRPipeline
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
			Mesh particle("square.obj");
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
	//	smoke->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 1.0f));
		smoke->head = jet;

		auto sphereMesh1 = std::make_shared<MeshWrapper>(disneyPipeline, sphere);
	//	sphereMesh1->textures.push_back(smokeTex);
		sphereMesh1->scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
		sphereMesh1->color = glm::vec4(1.0f, 0.2f, 0.8f, 1.0f);
	//	sphereMesh1->illuminated = true;

		auto sphereMesh2 = std::make_shared<MeshWrapper>(disneyPipeline, realSphere);
	//	sphereMesh2->textures.push_back(smokeTex);
		sphereMesh2->scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
		sphereMesh2->translation = glm::translate(glm::mat4(1.0f), glm::vec3(63.0199f, 12.4609f, 92.8054f));
		sphereMesh2->illuminated = true;
		sphereMesh2->color = glm::vec4(1.0f, 1.0f, 0.4f, 1.0f);




		auto mandelbulb = std::make_shared<MeshWrapper>(mandelbulbPipeline, square);
		mandelbulb->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 1.0f));
		mandelbulb->Billboard = true;
		mandelbulb->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);



		//auto quad = std::make_shared<MeshWrapper>(texturedPipeline, quadMesh);
		//quad->textures.push_back(smokeTex);



		//quad->scale = glm::scale(glm::mat4(1.0f), glm::vec3(20.0f, 20.0f, 20.0f));
		

		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;
		//meshWrappers.push_back(sun);
	//	meshWrappers.push_back(moon);
//		meshWrappers.push_back(globe);

		meshWrappers.push_back(jet);
		meshWrappers.push_back(sphereMesh1);
		meshWrappers.push_back(sphereMesh2);
		meshWrappers.push_back(skybox);
		//meshWrappers.push_back(smoke);

	//	meshWrappers.push_back(target);
	//	meshWrappers.push_back(woodbox);

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
	
		for (int i = 0; i < 150; i++)
		{
			meshWrappers.push_back(smoke);
		}
		create_mesh(meshWrappers);

		//m_Camera = Camera();
		//m_Camera.mesh = jet;

	}

	bool poll_inputs(GLFWwindow* window, float deltaTime) override
	{

		//float cameraSpeed = 100.0f;

		//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		//	m_Camera.update_position(cameraSpeed * deltaTime * m_Camera.Front);
		//}

		//if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		//}

		//if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//}

		//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		//	m_Camera.update_position(cameraSpeed * deltaTime * m_Camera.Right);
		//}

		//if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		//	imguiEnabled = !imguiEnabled;
		//}

		//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		//	m_Camera.update_position(-cameraSpeed * deltaTime * m_Camera.Front);
		//}

		//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		//	m_Camera.update_position(-cameraSpeed * deltaTime * m_Camera.Right);
		//}

		//if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		//{
		//	m_Camera.update_position(cameraSpeed * deltaTime * m_Camera.Up);
		//}


		//if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		//{
		//	m_Camera.update_position(-cameraSpeed * deltaTime * m_Camera.Up);
		//}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			return false;
		}

		return true;
	}

	void set_callbacks(GLFWwindow* window)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
			app->set_frame_buffer_resized();
			//TODO
			//app->set_frame_b

			});

		//glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		//	auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
		//	app->set_field_of_view(static_cast<float>(yoffset));
		//	//app->setScrollCallback();
		//	});


		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
			auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
			glm::vec2 mousePosition = app->get_mouse_position();

			app->set_mouse_position({ static_cast<float>(xpos), static_cast<float>(ypos) });
			
			std::cout << xpos << " " << ypos << std::endl;
			// app->process_mouse_movement(0.0f, 0.0f);
			});
	}

	virtual void draw_imgui(GLFWwindow* window) override
	{
		auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoBackground;
		window_flags |= ImGuiWindowFlags_NoTitleBar;

		ImVec2 windowSize{ 250, 350 };
		ImGui::SetNextWindowSize(windowSize);
		// etc.
		bool open_ptr = true;
		ImGui::Begin("I'm a Window!", &open_ptr, window_flags);
		
		auto& disneyParams = app->get_disney_params();

		if (ImGui::CollapsingHeader("Water Surface Settings",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);

			ImGui::DragFloat("subsurface", &disneyParams.subsurface, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("metallic", &disneyParams.metallic, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("specular", &disneyParams.specular, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("specularTint", &disneyParams.specularTint, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("roughness", &disneyParams.roughness, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("anisotropic", &disneyParams.anisotropic, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("sheen", &disneyParams.sheen, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("sheenTint", &disneyParams.sheenTint, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("clearcoat", &disneyParams.clearcoat, 0.01f, 0.0f, 1.0f, "%.2f");
			ImGui::DragFloat("clearcoatGloss", &disneyParams.clearcoatGloss, 0.01f, 0.0f, 1.0f, "%.2f");

			ImGui::PopItemWidth();
			ImGui::NewLine();
		}

		/*	if (ImGui::CollapsingHeader("Mesh Settings",
				ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
				ShowMeshSettings();
				ImGui::PopItemWidth();
				ImGui::NewLine();
			}*/

		ImGui::End();
	}

private:
	int mandelbulb_factor;
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

