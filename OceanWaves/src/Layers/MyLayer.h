#pragma once

#include "LayerInit.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


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
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL);

		auto image2dIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dOut2 = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2DFragment = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);

		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		auto totalTimeCompute = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);

		auto mandelbulbFactor = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::mandelbulbFactorUpdateFunc);
		auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight), Functions::globalLightUpdateFunc);

		create_descriptors({ camera,objects, resolution, totalTime, mandelbulbFactor, globalLight, waves,sampler, image2dIn, image2dOut, image2dOut2, image2DFragment, totalTimeCompute });

		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------

		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;

		TopoloG skyboxTopology({ camera, sampler });
		//TopoloG mandelbulbTopology({ camera, objects, resolution, totalTime, mandelbulbFactor });
		TopoloG oceanTopology({ camera, waves, totalTime, objects, globalLight });
		TopoloG plainTopology({ camera, objects });

		TopoloG topologyTex({ camera, objects, sampler });
		TopoloG imagefieldTopology({ camera, objects, globalLight, image2DFragment, image2dOut2 });

		TopoloG pleaseTop({ camera, totalTime, objects, sampler });

		TopoloG computeShaderTopology({ totalTimeCompute, image2dIn, image2dOut, image2dOut2 });

		auto skyboxLayout = std::make_shared<PipelineLayout>(skyboxTopology);
		auto oceanLayout = std::make_shared<PipelineLayout>(oceanTopology);
		auto plainLayout = std::make_shared<PipelineLayout>(plainTopology);
		auto pipelineLayoutTex = std::make_shared<PipelineLayout>(topologyTex);
		auto imageFieldPipelineLayout = std::make_shared<PipelineLayout>(imagefieldTopology);
		auto pleaseLayout = std::make_shared<PipelineLayout>(pleaseTop);

		auto computeLayout = std::make_shared<PipelineLayout>(computeShaderTopology);
		//auto mandelbulbLayout = std::make_shared<PipelineLayout>(mandelbulbTopology);

		create_layouts({ oceanLayout, plainLayout , pleaseLayout, computeLayout , pipelineLayoutTex, imageFieldPipelineLayout });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------


		std::vector<std::string> skyboxShaderNames({ "SkyboxShader.vert", "SkyboxShader.frag" });
		std::vector<std::string> oceanShaderNames({ "OceanShader.vert", "OceanShader.frag" });
		std::vector<std::string> fftShaderNames({ "FFTOceanShader.vert", "FFTOceanShader.frag" });
		std::vector<std::string> plainShaderNames({ "PlainShader.vert", "PlainShader.frag" });
		std::vector<std::string> computerShaderNames({ "FFTShader.comp" });
		std::vector<std::string> textureShaderNames({ "TextureShader.vert", "TextureShader.frag" });
		std::vector<std::string> imageFieldShaderNames({ "ImageFieldShader.vert", "ImageFieldShader.frag" });



		//------------------------------- PIPELINES ----------------------------------------------------------

		auto skyboxPipeline = std::make_shared<Pipeline>(skyboxLayout, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto oceanPipeline = std::make_shared<Pipeline>(oceanLayout, oceanShaderNames);
		auto plainPipeline = std::make_shared<Pipeline>(plainLayout, plainShaderNames);
		auto texturedPipeline = std::make_shared<Pipeline>(pipelineLayoutTex, textureShaderNames);
		auto imagefieldPipeline = std::make_shared<Pipeline>(imageFieldPipelineLayout, imageFieldShaderNames);
		auto fftPipeline = std::make_shared<Pipeline>(pleaseLayout, fftShaderNames);

		auto computePipeline = std::make_shared<Pipeline>(computeLayout, computerShaderNames);
		m_ComputePipeline = computePipeline;


		create_pipelines({oceanPipeline, plainPipeline, fftPipeline, computePipeline , texturedPipeline, imagefieldPipeline });

		//------------------------- TEXTURES ---------------------------------------------------------------

		std::shared_ptr<Texture> skyboxTex = std::make_shared<Texture>("nightbox/");	


		//-------------------------------------- IMAGE FIELDS --------------------------------------------

		int Lx = 1024;
		int Lz = 1024;

		std::shared_ptr<Texture> h0 = std::make_shared<Texture>(1024, [Lx, Lz, this](int width, int height, int channels) {
			float* pixels;
			pixels = (float*)malloc(width * height * channels * sizeof(float));

			// Generate pixel data with unique colors
			float avg_x = 0.0f;
			float avg_y = 0.0f;
			float delta_k = 2 * M_PI / Lx;
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x <  width; ++x) {
					int index = (y * width + x) * channels;

					int n = x - width / 2;
					int m = y - height / 2;

					float kx = 2 * M_PI * n / Lx;
					float kz = 2 * M_PI * m / Lz;

					//	std::cout << kx << " " << kz << std::endl;

					glm::vec2 K = glm::vec2(kx, kz);

					float k_len = glm::length(K);

					glm::vec3 h_0({ 0.0f, 0.0f, 0.0f });
					if (k_len > std::sqrt(2) * 2 * M_PI / Lx && k_len < M_PI)
					{
						float phi = std::atan(K.y / K.x);
						h_0 = this->fourier_amplitude(k_len, this->gen, this->distribution, phi, delta_k);
					//	std::cout << h_0.x << std::endl;
					}

					pixels[index + 0] = h_0.x;   // Red component
					pixels[index + 1] = h_0.y;  // Green component
					pixels[index + 2] = kx;  // Blue component (set to 0 for simplicity)
					pixels[index + 3] = kz;  // Alpha component (set to full alpha)
				}
			}

			std::cout << "VAL" << avg_x << " " << avg_y << std::endl;

			return pixels;
			});


		std::shared_ptr<Texture> hx = std::make_shared<Texture>(1024);
		std::shared_ptr<Texture> dh = std::make_shared<Texture>(1024);
		//waveHeightField->DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;


		create_image_fields({ h0, hx, dh });

		m_ComputePipeline->ImageFields.push_back(h0);
		m_ComputePipeline->ImageFields.push_back(hx);
		m_ComputePipeline->ImageFields.push_back(dh);

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

		auto globalLighter = std::make_shared<LightProperties>(LightType::GlobalLight, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));


		auto ocean = std::make_shared<MeshWrapper>(oceanPipeline, plain);
		//ocean->textures.push_back(waveHeightField);
		ocean->color = glm::vec4(0.0f, 0.1569f, 0.3922f, 1.0f);
		ocean->lightProperties = globalLighter;
		ocean->lightType = LightType::GlobalLight;
	//	ocean->Billboard = true;
		//ocean->scale = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 1.0f));



		auto woodbox = std::make_shared<MeshWrapper>(imagefieldPipeline, plain);
	//	woodbox->scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
		woodbox->Billboard = false;
		woodbox->image_fields.push_back(hx);
		woodbox->image_fields.push_back(dh);
		woodbox->color = glm::vec4(0.0f, 0.1569f, 0.3922f, 1.0f);
		//woodbox->lightType = LightType::GlobalLight;
		//woodbox->lightProperties = globalLighter;


		meshWrappers.push_back(ocean);
		meshWrappers.push_back(woodbox);
	//	meshWrappers.push_back(heightmap);

		create_mesh(meshWrappers);


	}

	int get_mandelbulb_factor() override { return this->mandelbulb_factor; }

	glm::vec3 fourier_amplitude(float k_len, std::mt19937& gen, std::normal_distribution<float>& distribution, float phi, float delta_k)
	{
		float g = 9.81f;
		float omega = pow(g * k_len, 0.5f);

	//	std::cout << omega << std::endl;

		float theta = std::atan(k_len);

		float fetch = 50000.0f;
		float u_10 = 10.0f;
		float energy = jonswap(omega, 200000, 20);

		float omega_p = 22 * pow(g * g / (u_10 * fetch), 0.333f); //angular frequency of the spectral peak


		energy *= oceanography_donelan_banner_directional_spreading(omega, omega_p, theta);

		float factor = 1 / pow(2, 0.5f) * pow(2 * energy, 0.5f) * delta_k;

		return glm::vec3(factor * distribution(gen), factor * distribution(gen), omega);
	}

	float directional_spread(float omega, float omega_p, float theta, float wind_direction)
	{
		float sp = (omega >= omega_p ? 9.77f : 6.97f);
		float s = sp * (omega >= omega_p ? pow(omega / omega_p, -2.5f) : pow(omega / omega_p, 5.0f));

		float factor1 = pow(2, 2 * s - 1) / glm::pi<float>();
		float factor2 = pow(std::tgamma(s+1), 2.0f) / std::tgamma(2*s + 1);
		float factor3 = pow( std::abs( cos( (theta-wind_direction) / 2) ), 2*s);
		return factor1 * factor2 * factor3;
	}

	float __oceanography_donelan_banner_beta_s(float omega, float omega_peak)
	{
		float om_over_omp = omega / omega_peak;
		float epsilon = -0.4f + 0.8393 * exp(-0.567f * pow(log(om_over_omp), 2.0f));
		float beta_s_0 = 2.61f * pow(om_over_omp, 1.3f);
		float beta_s_1 = 2.28f * pow(om_over_omp, -1.3f);
		float beta_s_2 = pow(10.0f, epsilon);
		return om_over_omp < 0.95f
			? beta_s_0
			: om_over_omp < 1.6f
			? beta_s_1
			: beta_s_2;
	}

	float math_sech(float x)
	{
		return 1.0f / cosh(x);
	}

	float oceanography_donelan_banner_directional_spreading(float omega, float omega_peak, float theta)
	{
		float beta_s = __oceanography_donelan_banner_beta_s(omega, omega_peak);
		return (beta_s / (2.0f * tanh(beta_s * M_PI))) * pow(math_sech(std::clamp(beta_s * theta, -9.0f, 9.0f)), 2.0f);
	}

	float jonswap(double  omega, float fetch, float U_10)
	{
		float g = 9.81f;

		float alpha = 0.076f * pow(pow(U_10, 2) / (fetch * g), 0.22f) * 1.3; // equilibrium range parameter

		float omega_p = 22 * pow(g * g / (U_10 * fetch), 0.333f); //angular frequency of the spectral peak

		float gamma = 3.3f;

		// Calculate the significant wave height
		double sigma = omega < omega_p ? 0.07 : 0.09; // Standard deviation for JONSWAP spectrum
		//  double omega_p = 2 * M_PI / Tp; // Peak angular frequency



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

