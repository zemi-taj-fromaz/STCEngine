#pragma once

#include "LayerInit.h"

class MyLayer : public Layer
{
public:
	MyLayer() : Layer("Example")
	{
		imguiEnabled = false;

		//------------------------------ DESCRIPTORS ---------------------------------------------------

		auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);
		auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
		auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		auto mandelbulbFactor = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::mandelbulbFactorUpdateFunc);

		create_descriptors({ camera,objects, resolution, sampler, totalTime, mandelbulbFactor });


		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------


		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;

		TopoloG skyboxTopology({ camera, sampler });
		TopoloG mandelbulbTopology({ camera, objects, resolution, totalTime, mandelbulbFactor });

		auto skyboxLayout = std::make_shared<PipelineLayout>(skyboxTopology);
		auto mandelbulbLayout = std::make_shared<PipelineLayout>(mandelbulbTopology);

		create_layouts({ skyboxLayout, mandelbulbLayout });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------


		std::vector<std::string> skyboxShaderNames({ "SkyboxShader.vert", "SkyboxShader.frag" });
		std::vector<std::string> mandelbulbShaderNames({ "MandelbulbShader.vert", "MandelbulbShader.frag" });


		//------------------------------- PIPELINES ----------------------------------------------------------

		auto skyboxPipeline = std::make_shared<Pipeline>(skyboxLayout, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto mandelbulbPipeline = std::make_shared<Pipeline>(mandelbulbLayout, mandelbulbShaderNames);

		create_pipelines({skyboxPipeline, mandelbulbPipeline });

		//------------------------- TEXTURES ---------------------------------------------------------------

		std::shared_ptr<Texture> skyboxTex = std::make_shared<Texture>("nightbox/");			


		create_textures({  skyboxTex });

		//----------------------- MESH --------------------------------------------------------------------
		Mesh box("skybox.obj");
		Mesh square("square.obj");
		Mesh cube(MeshType::Cube);
		Mesh quad(MeshType::Quad);

		//-------------------- LIGHTS -----------------------------------------------------------------------

		this->m_Camera = Camera();
		m_Camera.Position = glm::vec3(0.0f, 0.0f, 50.0f);

		//-------------------- MESH WRAPPERS ----------------------------------------------------------------

		auto skybox = std::make_shared<MeshWrapper>(skyboxPipeline, box);
		skybox->textures.push_back(skyboxTex);
		skybox->isSkybox = true;



		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;




		meshWrappers.push_back(skybox);

		std::random_device rd;
		std::mt19937 gen(rd());
		// Generate a random angle in radians
		std::uniform_real_distribution<> phiDistribution(0.0, 360.0f);
		std::uniform_real_distribution<> thetaDistribution(0.0, 360.0f);

		std::uniform_real_distribution<> distanceDistribution(0.0f, 500.0f);

		for (int i = 0; i < 50; i++)
		{
			double phi = phiDistribution(gen);
			double theta = thetaDistribution(gen);
			double distance = distanceDistribution(gen);

			double xOffset = distance * sin(phi) * cos(theta);
			double yOffset = distance * sin(phi) * sin(theta);
			double zOffset = distance * cos(theta);

			glm::vec3 offset = m_Camera.Position + glm::vec3(xOffset, yOffset, zOffset);

			auto mandelbulb = std::make_shared<MeshWrapper>(mandelbulbPipeline, square);
			mandelbulb->scale = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 1.0f));
			mandelbulb->Billboard = true;
			mandelbulb->translation = glm::translate(glm::mat4(1.0f), offset + m_Camera.Position);
	
			meshWrappers.push_back(mandelbulb);
		}

		create_mesh(meshWrappers);


	}

	virtual bool poll_inputs(GLFWwindow* window, float deltaTime) override
	{
		float cameraSpeed = 20.0f;

		auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));

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

		{
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				return false;
			}

			if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
				mandelbulb_factor = 9;
			}

			if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
				mandelbulb_factor = 1;
			}

			if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
				
				mandelbulb_factor = 2;
			}

			if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
				
				mandelbulb_factor = 3;
			}

			if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
				mandelbulb_factor = 4;
			}

			if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
				mandelbulb_factor = 5;
			}

			if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
				mandelbulb_factor = 6;
			}

			if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
				mandelbulb_factor = 7;
			}

			if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
				mandelbulb_factor = 8;
			}
		}

		return true;
	}

	int get_mandelbulb_factor() override { return this->mandelbulb_factor; }

private:
	
	int mandelbulb_factor = 8;

};

