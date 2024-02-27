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
		auto waves = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(WaveData) * 50, Functions::wavesUpdateFunc);
		auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		auto mandelbulbFactor = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::mandelbulbFactorUpdateFunc);
		auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight), Functions::globalLightUpdateFunc);

		create_descriptors({ camera,objects, resolution, sampler, totalTime, mandelbulbFactor, globalLight, waves });


		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------


		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;

		TopoloG skyboxTopology({ camera, sampler });
		//TopoloG mandelbulbTopology({ camera, objects, resolution, totalTime, mandelbulbFactor });
		TopoloG oceanTopology({ camera, waves, totalTime, objects, globalLight });
		TopoloG plainTopology({ camera, objects });

		auto skyboxLayout = std::make_shared<PipelineLayout>(skyboxTopology);
		auto oceanLayout = std::make_shared<PipelineLayout>(oceanTopology);
		auto plainLayout = std::make_shared<PipelineLayout>(plainTopology);
		//auto mandelbulbLayout = std::make_shared<PipelineLayout>(mandelbulbTopology);

		create_layouts({ skyboxLayout, oceanLayout, plainLayout });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------


		std::vector<std::string> skyboxShaderNames({ "SkyboxShader.vert", "SkyboxShader.frag" });
		std::vector<std::string> oceanShaderNames({ "OceanShader.vert", "OceanShader.frag" });
		std::vector<std::string> plainShaderNames({ "PlainShader.vert", "PlainShader.frag" });


		//------------------------------- PIPELINES ----------------------------------------------------------

		auto skyboxPipeline = std::make_shared<Pipeline>(skyboxLayout, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto oceanPipeline = std::make_shared<Pipeline>(oceanLayout, oceanShaderNames);
		auto plainPipeline = std::make_shared<Pipeline>(plainLayout, plainShaderNames);


		create_pipelines({skyboxPipeline, oceanPipeline, plainPipeline });

		//------------------------- TEXTURES ---------------------------------------------------------------

		std::shared_ptr<Texture> skyboxTex = std::make_shared<Texture>("nightbox/");			


	//	create_textures({  skyboxTex });

		//----------------------- MESH --------------------------------------------------------------------
		Mesh box("skybox.obj");
		Mesh square("square.obj");
		Mesh cube(MeshType::Cube);
		Mesh quad(MeshType::Quad);
		
		Mesh plain(MeshType::Plain);


		//-------------------- WAVES ------------------------------------------------------------------------


		std::random_device rd;
		std::mt19937 gen(rd());


			// Procedural Settings
		int waveCount = 8;
		float medianWavelength = 1.0f;
		float wavelengthRange = 1.0f;
		float medianDirection = 0.0f;
		float directionalRange = 30.0f;
		float medianAmplitude = 0.5f;
		float medianSpeed = 1.0f;
		float speedRange = 0.1f;
		float steepness = 0.0f;

		float wavelengthMin = medianWavelength / (1.0f + wavelengthRange);
		float wavelengthMax = medianWavelength * (1.0f + wavelengthRange);
		float directionMin = medianDirection - directionalRange;
		float directionMax = medianDirection + directionalRange;
		float speedMin = glm::max(0.01f, medianSpeed - speedRange);
		float speedMax = medianSpeed + speedRange;
		float ampOverLen = medianAmplitude / medianWavelength;

		float halfPlaneWidth = 400 * 0.5f;
		glm::vec3 minPoint = glm::vec3(-halfPlaneWidth, 0.0f, -halfPlaneWidth);
		glm::vec3 maxPoint = glm::vec3(halfPlaneWidth, 0.0f, halfPlaneWidth);




		std::uniform_real_distribution<float> wavelengthDis(wavelengthMin, wavelengthMax);
		std::uniform_real_distribution<float> directionDis(directionMin, directionMax);
		std::uniform_real_distribution<float> speedDis(speedMin, speedMax);
		std::uniform_real_distribution<float> originDis(0.0f, 800.0f);

		std::vector<std::shared_ptr<WaveData>> waveData;
		
		
		for (int wi = 0; wi < waveCount; ++wi) {
			float wavelength = wavelengthDis(gen);
			float direction = directionDis(gen);
			float amplitude = wavelength * ampOverLen;
			float speed = speedDis(gen);
			glm::vec2 origin = glm::vec2( originDis(gen), originDis(gen));

			waveData.push_back(std::make_shared<WaveData>(wavelength, amplitude, speed, direction, steepness, origin));
		}

		create_waves(waveData);

		//-------------------- LIGHTS -----------------------------------------------------------------------

		this->m_Camera = Camera();
		m_Camera.Position = glm::vec3(0.0f, 0.0f, 50.0f);

		//-------------------- MESH WRAPPERS ----------------------------------------------------------------

		auto skybox = std::make_shared<MeshWrapper>(skyboxPipeline, box);
		skybox->textures.push_back(skyboxTex);
		skybox->isSkybox = true;



		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;

		auto ocean = std::make_shared<MeshWrapper>(oceanPipeline, plain);
		ocean->illuminated = true;
		ocean->color = glm::vec4(0.1f, 0.0f, 1.0f, 1.0f);


		auto globalLighter = std::make_shared<LightProperties>(LightType::GlobalLight, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));

		auto woodbox = std::make_shared<MeshWrapper>(plainPipeline, cube);
		woodbox->lightType = LightType::GlobalLight;
		woodbox->lightProperties = globalLighter;

		meshWrappers.push_back(ocean);
		meshWrappers.push_back(woodbox);

		create_mesh(meshWrappers);


	}

	int get_mandelbulb_factor() override { return this->mandelbulb_factor; }

private:
	
	int mandelbulb_factor = 8;

};

