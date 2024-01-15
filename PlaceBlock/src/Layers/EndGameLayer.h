#pragma once
#pragma once

#include "LayerInit.h"

class EndGameLayer : public Layer
{
public:
	EndGameLayer() : Layer("Example")
	{
		imguiEnabled = false;
		//TODO GET THIS TO WORK

		//------------------------------ DESCRIPTORS ---------------------------------------------------
	//	auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);

		auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);


		create_descriptors({ resolution, sampler, objects //}, albedo, normals, metallic, roughness
			});// , deltaTime, ssboIn, ssboOut


		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------


		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;
		TopoloG buttonTopology({ objects, resolution, sampler });

		auto buttonLayout = std::make_shared<PipelineLayout>(buttonTopology);

		create_layouts({ buttonLayout });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------

		std::vector<std::string> buttonShaderNames({ "ButtonShader.vert", "ButtonShader.frag" });
		//std::vector<std::string> PBRShaderNames({ "PBRShader.vert", "PBRShader.frag" });

		//------------------------------- PIPELINES ----------------------------------------------------------

		auto buttonPipeline = std::make_shared<Pipeline>(buttonLayout, buttonShaderNames);
		buttonPipeline->cullMode = VK_CULL_MODE_NONE;

		create_pipelines({buttonPipeline});//, m_ComputePipeline, m_ComputeGraphicsPipeline});

		//------------------------- TEXTURES ---------------------------------------------------------------

		std::shared_ptr<Texture> gameOverTex = std::make_shared<Texture>("gameOver.jpg");
		std::shared_ptr<Texture> playAgainTex = std::make_shared<Texture>("playAgain.jpg");

		create_textures({ playAgainTex //}, rustAlbedoTex, rustNormalsTex, rustMetallicTex, rustRoughnessTex
			});

		//----------------------- MESH --------------------------------------------------------------------


		Mesh quadMesh(MeshType::Quad);

		//-------------------- LIGHTS -----------------------------------------------------------------------

		this->m_Camera = Camera();
		m_Camera.Position = glm::vec3(0.0f, 0.0f, 50.0f);




		auto gameOver = std::make_shared<MeshWrapper>(buttonPipeline, quadMesh);
		//gameOver->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 1.0f));
		//gameOver->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2f, 0.7f, 0.0f));
		gameOver->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		gameOver->rotation = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		gameOver->scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
		gameOver->textures.push_back(playAgainTex);

		auto playAgain = std::make_shared<MeshWrapper>(buttonPipeline, quadMesh);
		playAgain->translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.7f, 0.0f));
		playAgain->textures.push_back(playAgainTex);

		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;

		meshWrappers.push_back(gameOver);
	//	meshWrappers.push_back(playAgain);

		create_mesh(meshWrappers);
	}

	virtual void set_callbacks(GLFWwindow* window) override
	{

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
			app->set_frame_buffer_resized();
			});


		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
			auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
			glm::vec2 mousePosition = app->get_mouse_position();

			app->set_mouse_position({ static_cast<float>(xpos), static_cast<float>(ypos) });

			float xoffset = static_cast<float>(xpos) - mousePosition.x;
			float yoffset = mousePosition.y - static_cast<float>(ypos); // reversed since y-coordinates range from bottom to top

			const float sensitivity = 0.05f;
			xoffset *= sensitivity;
			yoffset *= sensitivity;

			app->process_mouse_movement(xoffset, yoffset);
			});
	}

	virtual bool poll_inputs(GLFWwindow* window, float deltaTime) override
	{

		auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			app->restart_game();

		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			return false;
		}

		return true;
	}

};

