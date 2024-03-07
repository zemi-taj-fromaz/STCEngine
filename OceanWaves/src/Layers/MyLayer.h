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

		auto samplerVertex = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

		auto image2dIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
		auto image2dOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
		auto image2dOut2 = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);

		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		auto totalTimeCompute = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);

		auto mandelbulbFactor = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::mandelbulbFactorUpdateFunc);
		auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight), Functions::globalLightUpdateFunc);
		//auto heightImageIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
		//auto heightImageOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT);
		//auto heightMapData = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(HeightMapData), Functions::heightMapDataUpdateFunc);

		create_descriptors({ camera,objects, resolution, totalTime, mandelbulbFactor, globalLight, waves, samplerVertex, image2dIn, image2dOut, image2dOut2, totalTimeCompute });

		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------

		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;

		TopoloG skyboxTopology({ camera, sampler });
		//TopoloG mandelbulbTopology({ camera, objects, resolution, totalTime, mandelbulbFactor });
		TopoloG oceanTopology({ camera, waves, totalTime, objects, globalLight });
		TopoloG plainTopology({ camera, objects });
		TopoloG topologyTex({ camera, objects, sampler });
		TopoloG pleaseTop({ camera, totalTime, objects, samplerVertex });

		TopoloG computeShaderTopology({ totalTimeCompute, image2dIn, image2dOut });

		auto skyboxLayout = std::make_shared<PipelineLayout>(skyboxTopology);
		auto oceanLayout = std::make_shared<PipelineLayout>(oceanTopology);
		auto plainLayout = std::make_shared<PipelineLayout>(plainTopology);
		auto pipelineLayoutTex = std::make_shared<PipelineLayout>(topologyTex);
		auto pleaseLayout = std::make_shared<PipelineLayout>(pleaseTop);

		auto computeLayout = std::make_shared<PipelineLayout>(computeShaderTopology);
		//auto mandelbulbLayout = std::make_shared<PipelineLayout>(mandelbulbTopology);

		create_layouts({ oceanLayout, plainLayout , pleaseLayout, computeLayout });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------


		std::vector<std::string> skyboxShaderNames({ "SkyboxShader.vert", "SkyboxShader.frag" });
		std::vector<std::string> oceanShaderNames({ "OceanShader.vert", "OceanShader.frag" });
		std::vector<std::string> fftShaderNames({ "FFTOceanShader.vert", "FFTOceanShader.frag" });
		std::vector<std::string> plainShaderNames({ "PlainShader.vert", "PlainShader.frag" });
		std::vector<std::string> computerShaderNames({ "FFTShader.comp" });
		std::vector<std::string> textureShaderNames({ "TextureShader.vert", "TextureShader.frag" });



		//------------------------------- PIPELINES ----------------------------------------------------------

		auto skyboxPipeline = std::make_shared<Pipeline>(skyboxLayout, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto oceanPipeline = std::make_shared<Pipeline>(oceanLayout, oceanShaderNames);
		auto plainPipeline = std::make_shared<Pipeline>(plainLayout, plainShaderNames);
		auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayoutTex, textureShaderNames);
		auto fftPipeline = std::make_shared<Pipeline>(pleaseLayout, fftShaderNames);

		auto computePipeline = std::make_shared<Pipeline>(computeLayout, computerShaderNames);
		m_ComputePipeline = computePipeline;


		create_pipelines({oceanPipeline, plainPipeline, fftPipeline, computePipeline });

		//------------------------- TEXTURES ---------------------------------------------------------------

		std::shared_ptr<Texture> skyboxTex = std::make_shared<Texture>("nightbox/");	


		//-------------------------------------- IMAGE FIELDS --------------------------------------------

		int Lx = 512;
		int Lz = 512;

		std::shared_ptr<Texture> h0 = std::make_shared<Texture>(256, [Lx, Lz, this](int width, int height, int channels) {
			unsigned char* pixels;
			pixels = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));

			// Generate pixel data with unique colors
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					int index = (y * width + x) * channels;

					int n = x - width / 2;
					int m = y - height / 2;

					float kx = 2 * M_PI * n / Lx;
					float kz = 2 * M_PI * m / Lz;

					//	std::cout << kx << " " << kz << std::endl;

					glm::vec2 K = glm::vec2(kx, kz);

					glm::vec3 h_cap = this->fourier_amplitude(K, this->gen, this->distribution);

					//pixels[index + 0] = (uint8_t)(h_cap.x * 255);   // Red component
					//pixels[index + 1] = (uint8_t)(h_cap.y * 255);  // Green component
					//pixels[index + 2] = 0;  // Blue component (set to 0 for simplicity)
					//pixels[index + 3] = 255;  // Alpha component (set to full alpha)

					//std::cout << h_cap.x << " " << h_cap.y << " " << 255 << std::endl;

					pixels[index + 0] = h_cap.x;   // Red component
					pixels[index + 1] = h_cap.y;  // Green component
					pixels[index + 2] = h_cap.z;  // Blue component (set to 0 for simplicity)
					pixels[index + 3] = 255;  // Alpha component (set to full alpha)
				}
			}

			return pixels;
			});

		std::shared_ptr<Texture> hk = std::make_shared<Texture>(256);

		std::shared_ptr<Texture> hx = std::make_shared<Texture>(512);
		std::shared_ptr<Texture> dh = std::make_shared<Texture>(512);
		//waveHeightField->DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;


		create_image_fields({ h0, hk, hx, dh });

		m_ComputePipeline->ImageFields.push_back(h0);
		m_ComputePipeline->ImageFields.push_back(hk);

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
		int waveCount = 32;
		float medianWavelength = 2.0f;
		float wavelengthRange = 1.0f;
		float medianDirection = 0.0f;
		float directionalRange = 180.0f;
		float medianAmplitude = 1.0f;
		float medianSpeed = 0.5f;
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
		
		float wavelength = 6.0f;
		float amplitude = 0.6f;

		float factor = 0.2;
		
		for (int wi = 0; wi < waveCount; ++wi) {
			//float wavelength = wavelengthDis(gen);
			float direction = directionDis(gen);
			//float amplitude = wavelength * ampOverLen;
			float speed = speedDis(gen);
			glm::vec2 origin = glm::vec2( originDis(gen), originDis(gen));

			waveData.push_back(std::make_shared<WaveData>(wavelength, amplitude, speed, direction, steepness, origin));

			wavelength *=  1/(1.0f + factor);
			amplitude *= (1.0f - factor);
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
		//ocean->textures.push_back(waveHeightField);
		ocean->color = glm::vec4(0.0f, 0.1569f, 0.3922f, 1.0f);
	//	ocean->Billboard = true;
		//ocean->scale = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 1.0f));


		auto globalLighter = std::make_shared<LightProperties>(LightType::GlobalLight, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));

		auto woodbox = std::make_shared<MeshWrapper>(plainPipeline, cube);
		woodbox->scale = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
		//woodbox->lightType = LightType::GlobalLight;
		//woodbox->lightProperties = globalLighter;

		auto heightmap = std::make_shared<MeshWrapper>(texturedPipeline, quad);
		heightmap->scale = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 1.0f));
		heightmap->Billboard = true;
		heightmap->textures.push_back(h0);

		meshWrappers.push_back(ocean);
		meshWrappers.push_back(woodbox);
	//	meshWrappers.push_back(heightmap);

		create_mesh(meshWrappers);


	}

	int get_mandelbulb_factor() override { return this->mandelbulb_factor; }

	glm::vec3 fourier_amplitude(glm::vec2 k, std::mt19937& gen, std::normal_distribution<float>& distribution)
	{
		float k_len = glm::length(k);
		float omega = pow(9.81f * k_len, 0.5f);

	//	std::cout << omega << std::endl;

		float derivatives = 1.0f;

		float directionalSpread = 1.0f;

		float factor = 1 / pow(2, 0.5f) * pow(jonswap(256, omega, 200) * directionalSpread * derivatives, 0.5f);

		return glm::vec3(factor * distribution(gen), factor * distribution(gen), omega);
	}

	float jonswap(int N, double  omega, float fetch)
	{
		float g = 9.81f;

		float U_10 = 10; //wind speed in meters per second at a heigh of ten meters above the sea surface

		float alpha = 0.076f * pow(pow(U_10, 2) / (fetch * g), 0.22f); // equilibrium range parameter

		float omega_p = 22 * pow(g * g / (U_10 * fetch), 0.333f); //angular frequency of the spectral peak

		float gamma = 3.3f;

		// Calculate the significant wave height
		double sigma = omega < omega_p ? 0.07 : 0.09; // Standard deviation for JONSWAP spectrum
		//  double omega_p = 2 * M_PI / Tp; // Peak angular frequency

		if(omega > omega_p)
		{
			std::cout << "OO" << std::endl;
		}


		float beta = 1.25f;
		float expr1 = pow(g, 2) / pow(omega, 5);
		float expr2 = exp(-beta * pow(omega_p / omega, 4));
		double S = alpha * expr1 * expr2;

		float r = exp(-pow(omega - omega_p, 2) / (2 * pow(sigma * omega_p, 2)));

		float peakEnhancementFactor = pow(gamma, r);

		float energy = S * peakEnhancementFactor;

		return energy;
	}

	std::random_device rd;
	std::mt19937 gen{ rd() };
	std::normal_distribution<float> distribution{ 0.0f, 1.0f };
private:
	int mandelbulb_factor = 8;

};

