#pragma once

#include "LayerInit.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

#include <fftw3.h>
#include <complex>

class MyLayer : public Layer
{
public:

	static constexpr uint32_t     s_kDefaultTileSize{ 256 };
	static constexpr float        s_kDefaultTileLength{ 256.0f };

	static inline const glm::vec2 s_kDefaultWindDir{ 1.0f, 1.0f };
	static constexpr float        s_kDefaultWindSpeed{ 30.0f };
	static constexpr float        s_kDefaultAnimPeriod{ 200.0f };
	static constexpr float        s_kDefaultPhillipsConst{ 3e-7f };
	static constexpr float        s_kDefaultPhillipsDamping{ 0.1f };

	// Both vec4 due to GPU memory alignment requirements
	using Displacement = glm::vec4;
	using Normal = glm::vec4;
	using Complex = std::complex<float>;

	struct WaveVector
	{
		glm::vec2 vec;
		glm::vec2 unit;

		WaveVector(glm::vec2 v)
			: vec(v),
			unit(glm::length(v) > 0.00001f ? glm::normalize(v) : glm::vec2(0))
		{}

		WaveVector(glm::vec2 v, glm::vec2 u)
			: vec(v), unit(u) {}
	};

	struct BaseWaveHeight
	{
		Complex heightAmp;          ///< FT amplitude of wave height
		Complex heightAmp_conj;     ///< conjugate of wave height amplitude
		float dispersion;           ///< Descrete dispersion value
	};


	uint32_t m_TileSize;
	float    m_TileLength;

	glm::vec2 m_WindDir;        ///< Unit vector
	float m_WindSpeed;

	// Phillips spectrum
	float m_A;
	float m_Damping;

	float m_AnimationPeriod;
	float m_BaseFreq{ 1.0f };

	float m_Lambda{ -1.0f };  ///< Importance of displacement vector

	// -------------------------------------------------------------------------
	// Data

	// vec4(Displacement_X, height, Displacement_Z, [jacobian])
	std::vector<Displacement> m_Displacements;
	// vec4(slopeX, slopeZ, dDxdx, dDzdz )
	std::vector<Normal> m_Normals;

	// =========================================================================
	// Computation

	std::vector<WaveVector> m_WaveVectors;   ///< Precomputed Wave vectors

	// Base wave height field generated from the spectrum for each wave vector
	std::vector<BaseWaveHeight> m_BaseWaveHeights;

	// ---------------------------------------------------------------------
	// FT computation using FFTW

	Complex* m_Height{ nullptr };
	Complex* m_SlopeX{ nullptr };
	Complex* m_SlopeZ{ nullptr };
	Complex* m_DisplacementX{ nullptr };
	Complex* m_DisplacementZ{ nullptr };
	Complex* m_dxDisplacementX{ nullptr };
	Complex* m_dzDisplacementZ{ nullptr };
#ifdef COMPUTE_JACOBIAN
	Complex* m_dxDisplacementZ{ nullptr };
	Complex* m_dzDisplacementX{ nullptr };
#endif

	fftwf_plan m_PlanHeight{ nullptr };
	fftwf_plan m_PlanSlopeX{ nullptr };
	fftwf_plan m_PlanSlopeZ{ nullptr };
	fftwf_plan m_PlanDisplacementX{ nullptr };
	fftwf_plan m_PlanDisplacementZ{ nullptr };
	fftwf_plan m_PlandxDisplacementX{ nullptr };
	fftwf_plan m_PlandzDisplacementZ{ nullptr };
#ifdef COMPUTE_JACOBIAN
	fftwf_plan m_PlandxDisplacementZ{ nullptr };
	fftwf_plan m_PlandzDisplacementX{ nullptr };
#endif

	float m_MinHeight{ -1.0f };
	float m_MaxHeight{ 1.0f };

	static constexpr float s_kG{ 9.81 };   ///< Gravitational constant
	const float s_kOneOver2sqrt{ std::sqrt(0.5f) }; // 1/sqrt(2)

	/**
	 * @brief Realization of water wave height field in fourier domain
	 * @return Fourier amplitudes of a wave height field
	 */
	Complex BaseWaveHeightFT(const Complex gaussRandom,
		const glm::vec2 unitWaveVec,
		float k) const
	{
		return s_kOneOver2sqrt * gaussRandom *
			glm::sqrt(PhillipsSpectrum(unitWaveVec, k));
	}

	/**
	 * @brief Phillips spectrum - wave spectrum,
	 *  a model for wind-driven waves larger than capillary waves
	 */
	float PhillipsSpectrum(const glm::vec2 unitWaveVec, float k) const
	{
		const float k2 = k * k;
		const float k4 = k2 * k2;

		float cosFact = glm::dot(unitWaveVec, m_WindDir);
		cosFact = cosFact * cosFact;

		const float L = m_WindSpeed * m_WindSpeed / s_kG;
		const float L2 = L * L;

		return m_A * glm::exp(-1.0f / (k2 * L2)) / k4
			* cosFact
			* glm::exp(-k2 * m_Damping * m_Damping);
	}

	static Complex WaveHeightFT(const BaseWaveHeight& waveHeight, const float t)
	{
		const float omega_t = waveHeight.dispersion * t;

		// exp(ix) = cos(x) * i*sin(x)
		const float pcos = glm::cos(omega_t);
		const float psin = glm::sin(omega_t);

		return waveHeight.heightAmp * Complex(pcos, psin) +
			waveHeight.heightAmp_conj * Complex(pcos, -psin);
	}

	float QDispersion(float k) const
	{
		return glm::floor(DispersionDeepWaves(k) / m_BaseFreq) * m_BaseFreq;
	}

	float DispersionDeepWaves(float k) const
	{
		return glm::sqrt(s_kG * k);
	}

	// ---------------------------------------------------------------------
// Getters

	auto GetTileSize() const { return m_TileSize; }
	auto GetTileLength() const { return m_TileLength; }
	auto GetWindDir() const { return m_WindDir; }
	auto GetWindSpeed() const { return m_WindSpeed; }
	auto GetAnimationPeriod() const { return m_AnimationPeriod; }
	auto GetPhillipsConst() const { return m_A; }
	auto GetDamping() const { return m_Damping; }
	auto GetDisplacementLambda() const { return m_Lambda; }
	float GetMinHeight() const { return m_MinHeight; }
	float GetMaxHeight() const { return m_MaxHeight; }


	size_t GetDisplacementCount() const { return m_Displacements.size(); }
	const std::vector<Displacement>& GetDisplacements() const {
		return m_Displacements;
	}

	size_t GetNormalCount() const { return m_Normals.size(); }
	const std::vector<Normal>& GetNormals() const { return m_Normals; }

	// ---------------------------------------------------------------------
	void SetTileSize(uint32_t size)
	{
		const bool kSizeIsPowerOfTwo = (size & (size - 1)) == 0;

		if (!kSizeIsPowerOfTwo)
			return;

		m_TileSize = size;
	}

	void SetTileLength(float length)
	{
		m_TileLength = length;
	}

	void SetWindDirection(const glm::vec2& w)
	{
		m_WindDir = glm::normalize(w);
	}

	void SetWindSpeed(float v)
	{
		m_WindSpeed = glm::max(0.0001f, v);
	}

	void SetAnimationPeriod(float T)
	{
		m_AnimationPeriod = T;
		m_BaseFreq = 2.0f * M_PI / m_AnimationPeriod;
	}

	void SetPhillipsConst(float A)
	{
		m_A = A;
	}

	void SetLambda(float lambda)
	{
		m_Lambda = lambda;
	}

	void SetDamping(float damping)
	{
		m_Damping = damping;
	}



	MyLayer(uint32_t tileSize, float tileLength) : Layer("Example")
	{

		SetTileSize(tileSize);
		SetTileLength(tileLength);

		SetWindDirection(s_kDefaultWindDir);
		SetWindSpeed(s_kDefaultWindSpeed);
		SetAnimationPeriod(s_kDefaultAnimPeriod);

		SetPhillipsConst(s_kDefaultPhillipsConst);
		SetDamping(s_kDefaultPhillipsDamping);

		m_WaveVectors = ComputeWaveVectors();

		std::vector<Complex> gaussRandomArr = ComputeGaussRandomArray();
		m_BaseWaveHeights = ComputeBaseWaveHeightField(gaussRandomArr);

		const uint32_t kSize = m_TileSize;

		const Displacement kDefaultDisplacement{ 0.0 };
		m_Displacements.resize(kSize * kSize, kDefaultDisplacement);

		const Normal kDefaultNormal{ 0.0, 1.0, 0.0, 0.0 };
		m_Normals.resize(kSize * kSize, kDefaultNormal);

		DestroyFFTW();
		SetupFFTW();

		imguiEnabled = false;

		//------------------------------ DESCRIPTORS ---------------------------------------------------

		auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);
		auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
		auto waves = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(WaveData) * 50, Functions::wavesUpdateFunc);
		auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);

		auto samplerVertex = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL);

		auto image2dIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dIn2 = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dOut2 = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2DFragment = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);

		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		auto totalTimeCompute = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);

		auto verticalFlag = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(bool), Functions::verticalFlagUpdateFunc);


		auto waterSurfaceUBO = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(WaterSurfaceUBO), Functions::surfaceUpdateFunc);

		auto mandelbulbFactor = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::mandelbulbFactorUpdateFunc);
		auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight), Functions::globalLightUpdateFunc);

		create_descriptors({ verticalFlag, camera, waterSurfaceUBO, objects, resolution, totalTime, mandelbulbFactor, globalLight, waves,sampler, image2dIn, image2dIn2, image2dOut, image2dOut2, image2DFragment, totalTimeCompute });

		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------

		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;

		TopoloG skyboxTopology({ camera, sampler });
		//TopoloG mandelbulbTopology({ camera, objects, resolution, totalTime, mandelbulbFactor });
		TopoloG oceanTopology({ camera, waves, totalTime, objects, globalLight });
		TopoloG plainTopology({ camera, objects });

		TopoloG topologyTex({ camera, objects, sampler });
		TopoloG imagefieldTopology({ camera, objects, globalLight, waterSurfaceUBO, image2DFragment, image2dOut2 });

		TopoloG pleaseTop({ camera, totalTime, objects, sampler });

		TopoloG computeShaderTopology({ totalTimeCompute, verticalFlag, image2dIn, image2dOut, image2dOut2 });
		TopoloG computeShaderTopology2({ totalTimeCompute, image2dIn, image2dIn2, image2dOut });

		auto skyboxLayout = std::make_shared<PipelineLayout>(skyboxTopology);
		auto oceanLayout = std::make_shared<PipelineLayout>(oceanTopology);
		auto plainLayout = std::make_shared<PipelineLayout>(plainTopology);
		auto pipelineLayoutTex = std::make_shared<PipelineLayout>(topologyTex);
		auto imageFieldPipelineLayout = std::make_shared<PipelineLayout>(imagefieldTopology);
		auto pleaseLayout = std::make_shared<PipelineLayout>(pleaseTop);

		auto computeLayout = std::make_shared<PipelineLayout>(computeShaderTopology);
		auto computeLayout2 = std::make_shared<PipelineLayout>(computeShaderTopology2);
		//auto mandelbulbLayout = std::make_shared<PipelineLayout>(mandelbulbTopology);

		create_layouts({ oceanLayout, plainLayout , pleaseLayout, computeLayout , pipelineLayoutTex, imageFieldPipelineLayout, computeLayout2 });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------


		std::vector<std::string> skyboxShaderNames({ "SkyboxShader.vert", "SkyboxShader.frag" });
		std::vector<std::string> oceanShaderNames({ "OceanShader.vert", "OceanShader.frag" });
		std::vector<std::string> fftShaderNames({ "FFTOceanShader.vert", "FFTOceanShader.frag" });
		std::vector<std::string> plainShaderNames({ "PlainShader.vert", "PlainShader.frag" });
		std::vector<std::string> computerShaderNames({ "FFTShader.comp" });
		std::vector<std::string> frequencyFieldShaderNames({ "FrequencyFieldShader.comp" });
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
		auto computePipeline2 = std::make_shared<Pipeline>(computeLayout2, frequencyFieldShaderNames);
		m_ComputePipeline = computePipeline;
		m_ComputePipeline2 = computePipeline2;


		create_pipelines({oceanPipeline, plainPipeline, fftPipeline, computePipeline , computePipeline2, texturedPipeline, imagefieldPipeline });

		//------------------------- TEXTURES ---------------------------------------------------------------


		std::shared_ptr<Texture> hx = std::make_shared<Texture>(tileLength, tileLength);
		std::shared_ptr<Texture> dh = std::make_shared<Texture>(tileLength, tileLength);
		//waveHeightField->DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;


		create_image_fields({hx, dh});
		//----------------------- MESH -------------------------------------------------------------------

		Mesh plain(MeshType::Plain);


		//-------------------- LIGHTS -----------------------------------------------------------------------

		this->m_Camera = Camera();
		m_Camera.Position = glm::vec3(0.0f, 50.0f, 50.0f);

		//-------------------- MESH WRAPPERS ----------------------------------------------------------------

		//auto skybox = std::make_shared<MeshWrapper>(skyboxPipeline, box);
		//skybox->textures.push_back(skyboxTex);
		//skybox->isSkybox = true;

		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;

		auto woodbox = std::make_shared<MeshWrapper>(imagefieldPipeline, plain);
		woodbox->Billboard = false;
		woodbox->image_fields.push_back(hx);
		woodbox->image_fields.push_back(dh);
		woodbox->color = glm::vec4(0.0f, 0.1569f, 0.3922f, 1.0f);


		meshWrappers.push_back(woodbox);

		create_mesh(meshWrappers);
	}

	float ComputeWaves(float t)
	{
		const auto kTileSize = m_TileSize;

		float masterMaxHeight = std::numeric_limits<float>::min();
		float masterMinHeight = std::numeric_limits<float>::max();

#pragma omp parallel shared(masterMaxHeight, masterMinHeight)
		{
#pragma omp for collapse(2) schedule(static)
			for (uint32_t m = 0; m < kTileSize; ++m)
				for (uint32_t n = 0; n < kTileSize; ++n)
				{
					m_Height[m * kTileSize + n] =
						WaveHeightFT(m_BaseWaveHeights[m * kTileSize + n], t);
				}

			// Slopes for normals computation
#pragma omp for collapse(2) schedule(static) nowait
			for (uint32_t m = 0; m < kTileSize; ++m)
				for (uint32_t n = 0; n < kTileSize; ++n)
				{
					const uint32_t kIndex = m * kTileSize + n;

					const auto& kWaveVec = m_WaveVectors[kIndex].vec;
					m_SlopeX[kIndex] = Complex(0, kWaveVec.x) * m_Height[kIndex];
					m_SlopeZ[kIndex] = Complex(0, kWaveVec.y) * m_Height[kIndex];
				}

			// Displacement vectors
#pragma omp for collapse(2) schedule(static)
			for (uint32_t m = 0; m < kTileSize; ++m)
				for (uint32_t n = 0; n < kTileSize; ++n)
				{
					const uint32_t kIndex = m * kTileSize + n;

					const auto& kWaveVec = m_WaveVectors[kIndex];
					m_DisplacementX[kIndex] = Complex(0, -kWaveVec.unit.x) *
						m_Height[kIndex];
					m_DisplacementZ[kIndex] = Complex(0, -kWaveVec.unit.y) *
						m_Height[kIndex];
					m_dxDisplacementX[kIndex] = Complex(0, kWaveVec.vec.x) *
						m_DisplacementX[kIndex];
					m_dzDisplacementZ[kIndex] = Complex(0, kWaveVec.vec.y) *
						m_DisplacementZ[kIndex];
#ifdef COMPUTE_JACOBIAN
					m_dzDisplacementX[kIndex] = Complex(0, kWaveVec.vec.y) *
						m_DisplacementX[kIndex];
					m_dxDisplacementZ[kIndex] = Complex(0, kWaveVec.vec.x) *
						m_DisplacementZ[kIndex];
#endif
				}

#pragma omp sections
			{
#pragma omp section
				{
					fftwf_execute(m_PlanHeight);
				}
#pragma omp section
				{
					fftwf_execute(m_PlanSlopeX);
				}
#pragma omp section
				{
					fftwf_execute(m_PlanSlopeZ);
				}
#pragma omp section
				{
					fftwf_execute(m_PlanDisplacementX);
				}
#pragma omp section
				{
					fftwf_execute(m_PlanDisplacementZ);
				}
#pragma omp section
				{
					fftwf_execute(m_PlandxDisplacementX);
				}
#pragma omp section
				{
					fftwf_execute(m_PlandzDisplacementZ);
				}
#ifdef COMPUTE_JACOBIAN
#pragma omp section
				{
					fftwf_execute(m_PlandzDisplacementX);
				}
#pragma omp section
				{
					fftwf_execute(m_PlandxDisplacementZ);
				}
#endif
			}

			float maxHeight = std::numeric_limits<float>::min();
			float minHeight = std::numeric_limits<float>::max();

			// Conversion of the grid back to interval
			//  [-m_TileSize/2, ..., 0, ..., m_TileSize/2]
			const float kSigns[] = { 1.0f, -1.0f };

#pragma omp for collapse(2) schedule(static) nowait
			for (uint32_t m = 0; m < kTileSize; ++m)
			{
				for (uint32_t n = 0; n < kTileSize; ++n)
				{
					const uint32_t kIndex = m * kTileSize + n;
					const int sign = kSigns[(n + m) & 1];
					const auto h_FT = m_Height[kIndex].real() * static_cast<float>(sign);
					maxHeight = glm::max(h_FT, maxHeight);
					minHeight = glm::min(h_FT, minHeight);

					auto& displacement = m_Displacements[kIndex];
					displacement.y = h_FT;
					displacement.x =
						static_cast<float>(sign) * m_Lambda * m_DisplacementX[kIndex].real();
					displacement.z =
						static_cast<float>(sign) * m_Lambda * m_DisplacementZ[kIndex].real();
					displacement.w = 1.0f;
				}
			}
			// TODO reduction
#pragma omp critical
			{
				masterMaxHeight = glm::max(maxHeight, masterMaxHeight);
				masterMinHeight = glm::min(minHeight, masterMinHeight);
			}

#pragma omp for collapse(2) schedule(static) nowait
			for (uint32_t m = 0; m < kTileSize; ++m)
			{
				for (uint32_t n = 0; n < kTileSize; ++n)
				{
					const uint32_t kIndex = m * kTileSize + n;
					const int sign = kSigns[(n + m) & 1];
#ifdef COMPUTE_JACOBIAN
					const float jacobian =
						(1.0f + m_Lambda * sign * m_dxDisplacementX[kIndex].real()) *
						(1.0f + m_Lambda * sign * m_dzDisplacementZ[kIndex].real()) -
						(m_Lambda * sign * m_dxDisplacementZ[kIndex].real()) *
						(m_Lambda * sign * m_dzDisplacementX[kIndex].real());
					displacement.w = jacobian;
#endif

					m_Normals[kIndex] = glm::vec4(
						sign * m_SlopeX[kIndex].real(),
						sign * m_SlopeZ[kIndex].real(),
						sign * m_dxDisplacementX[kIndex].real(),
						sign * m_dzDisplacementZ[kIndex].real()
					);
				}
			}
		}

		return NormalizeHeights(masterMinHeight, masterMaxHeight);
	}

	float NormalizeHeights(float minHeight, float maxHeight)
	{
		m_MinHeight = minHeight;
		m_MaxHeight = maxHeight;

		const float A = glm::max(glm::abs(minHeight), glm::abs(maxHeight));
		const float OneOverA = 1.f / A;

		std::for_each(m_Displacements.begin(), m_Displacements.end(),
			[OneOverA](auto& d) { d.y *= OneOverA; });

		return A;
	}

	void SetupFFTW()
	{

		const uint32_t kSize = m_TileSize;
		const uint32_t kSize2 = kSize * kSize;

#ifndef COMPUTE_JACOBIAN
		const uint32_t kTotalInputs = 7;
#else
		const uint32_t kTotalInputs = 7 + 2;
#endif

		m_Height = (Complex*)fftwf_alloc_complex(kTotalInputs * kSize2);

		m_SlopeX = m_Height + kSize2;
		m_SlopeZ = m_SlopeX + kSize2;
		m_DisplacementX = m_SlopeZ + kSize2;
		m_DisplacementZ = m_DisplacementX + kSize2;
		m_dxDisplacementX = m_DisplacementZ + kSize2;
		m_dzDisplacementZ = m_dxDisplacementX + kSize2;
#ifdef COMPUTE_JACOBIAN
		m_dzDisplacementX = m_dzDisplacementZ + kSize2;
		m_dxDisplacementZ = m_dzDisplacementX + kSize2;
#endif

#ifdef CAREFUL_ALLOC
		m_Height = (Complex*)fftwf_alloc_complex(kSize * kSize);
		m_SlopeX = (Complex*)fftwf_alloc_complex(kSize * kSize);
		m_SlopeZ = (Complex*)fftwf_alloc_complex(kSize * kSize);
		m_DisplacementX = (Complex*)fftwf_alloc_complex(kSize * kSize);
		m_DisplacementZ = (Complex*)fftwf_alloc_complex(kSize * kSize);
		m_dxDisplacementX = (Complex*)fftwf_alloc_complex(kSize * kSize);
		m_dzDisplacementZ = (Complex*)fftwf_alloc_complex(kSize * kSize);
#ifdef COMPUTE_JACOBIAN
		m_dzDisplacementX = (Complex*)fftwf_alloc_complex(kSize * kSize);
		m_dxDisplacementZ = (Complex*)fftwf_alloc_complex(kSize * kSize);
#endif
#endif

		m_PlanHeight = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_Height),
			reinterpret_cast<fftwf_complex*>(m_Height),
			FFTW_BACKWARD,
			FFTW_MEASURE);
		m_PlanSlopeX = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_SlopeX),
			reinterpret_cast<fftwf_complex*>(m_SlopeX),
			FFTW_BACKWARD,
			FFTW_MEASURE);
		m_PlanSlopeZ = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_SlopeZ),
			reinterpret_cast<fftwf_complex*>(m_SlopeZ),
			FFTW_BACKWARD,
			FFTW_MEASURE);
		m_PlanDisplacementX = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_DisplacementX),
			reinterpret_cast<fftwf_complex*>(m_DisplacementX),
			FFTW_BACKWARD,
			FFTW_MEASURE);
		m_PlanDisplacementZ = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_DisplacementZ),
			reinterpret_cast<fftwf_complex*>(m_DisplacementZ),
			FFTW_BACKWARD,
			FFTW_MEASURE);
		m_PlandxDisplacementX = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_dxDisplacementX),
			reinterpret_cast<fftwf_complex*>(m_dxDisplacementX),
			FFTW_BACKWARD,
			FFTW_MEASURE);
		m_PlandzDisplacementZ = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_dzDisplacementZ),
			reinterpret_cast<fftwf_complex*>(m_dzDisplacementZ),
			FFTW_BACKWARD,
			FFTW_MEASURE);
#ifdef COMPUTE_JACOBIAN
		m_PlandzDisplacementX = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_dzDisplacementX),
			reinterpret_cast<fftwf_complex*>(m_dzDisplacementX),
			FFTW_BACKWARD,
			FFTW_MEASURE);
		m_PlandxDisplacementZ = fftwf_plan_dft_2d(
			kSize, kSize,
			reinterpret_cast<fftwf_complex*>(m_dxDisplacementZ),
			reinterpret_cast<fftwf_complex*>(m_dxDisplacementZ),
			FFTW_BACKWARD,
			FFTW_MEASURE);
#endif
	}

	void DestroyFFTW()
	{

		if (m_PlanHeight == nullptr)
			return;

		fftwf_destroy_plan(m_PlanHeight);
		m_PlanHeight = nullptr;
		fftwf_destroy_plan(m_PlanSlopeX);
		fftwf_destroy_plan(m_PlanSlopeZ);
		fftwf_destroy_plan(m_PlanDisplacementX);
		fftwf_destroy_plan(m_PlanDisplacementZ);
		fftwf_destroy_plan(m_PlandxDisplacementX);
		fftwf_destroy_plan(m_PlandzDisplacementZ);
#ifdef COMPUTE_JACOBIAN
		fftwf_destroy_plan(m_PlandxDisplacementZ);
		fftwf_destroy_plan(m_PlandzDisplacementX);
#endif

		fftwf_free((fftwf_complex*)m_Height);
#ifdef CAREFUL_ALLOC
		fftwf_free((fftwf_complex*)m_SlopeX);
		fftwf_free((fftwf_complex*)m_SlopeZ);
		fftwf_free((fftwf_complex*)m_DisplacementX);
		fftwf_free((fftwf_complex*)m_DisplacementZ);
		fftwf_free((fftwf_complex*)m_dxDisplacementX);
		fftwf_free((fftwf_complex*)m_dzDisplacementZ);
#ifdef COMPUTE_JACOBIAN
		fftwf_free((fftwf_complex*)m_dxDisplacementZ);
		fftwf_free((fftwf_complex*)m_dzDisplacementX);
#endif
#endif
	}

	std::vector<BaseWaveHeight> ComputeBaseWaveHeightField(const std::vector<Complex>& gaussRandomArray) const
	{
		const uint32_t kSize = m_TileSize;

		std::vector<BaseWaveHeight> baseWaveHeights(kSize * kSize);
		assert(m_WaveVectors.size() == baseWaveHeights.size());
		assert(baseWaveHeights.size() == gaussRandomArray.size());

#pragma omp parallel for collapse(2) schedule(guided)
		for (uint32_t m = 0; m < kSize; ++m)
		{
			for (uint32_t n = 0; n < kSize; ++n)
			{
				const uint32_t kIndex = m * kSize + n;
				const auto& kWaveVec = m_WaveVectors[kIndex];
				const float k = glm::length(kWaveVec.vec);

				auto& h0 = baseWaveHeights[kIndex];
				if (k > 0.00001f)
				{
					const auto gaussRandom = gaussRandomArray[kIndex];
					h0.heightAmp =
						BaseWaveHeightFT(gaussRandom, kWaveVec.unit, k);
					h0.heightAmp_conj = std::conj(
						BaseWaveHeightFT(gaussRandom, -kWaveVec.unit, k));
					h0.dispersion = QDispersion(k);
				}
				else
				{
					h0.heightAmp = Complex(0);
					h0.heightAmp_conj = std::conj(Complex(0));
					h0.dispersion = 0.0f;
				}
			}
		}

		return baseWaveHeights;
	}

	std::vector<Complex> ComputeGaussRandomArray() const
	{
		const uint32_t kSize = m_TileSize;
		std::vector<Complex> randomArr(kSize * kSize);

		for (int32_t m = 0; m < kSize; ++m)
			for (int32_t n = 0; n < kSize; ++n)
			{
				randomArr[m * kSize + n] = Complex(glm::gaussRand(0.0f, 1.0f),
					glm::gaussRand(0.0f, 1.0f));
			}

		return randomArr;
	}

	std::vector<WaveVector> ComputeWaveVectors() const
	{
		const int32_t kSize = m_TileSize;
		const float kLength = m_TileLength;

		std::vector<WaveVector> waveVecs;
		waveVecs.reserve(kSize * kSize);

		for (int32_t m = 0; m < kSize; ++m)
		{
			for (int32_t n = 0; n < kSize; ++n)
			{
				waveVecs.emplace_back(
					glm::vec2(
						M_PI * (2.0f * n - kSize) / kLength,
						M_PI * (2.0f * m - kSize) / kLength
					)
				);
			}
		}

		return waveVecs;
	}

	virtual void compute_shaders_dispatch(VkCommandBuffer commandBuffer, uint32_t imageIndex, AppVulkanImpl* app) override 
	{
		float time = app->get_total_time();
		ComputeWaves(time);

		auto& hx = get_image_fields()[0];
		int width = hx->Width;
		int height = hx->Height;
		VkDeviceSize imageSize = hx->Width * hx->Height * 4 * sizeof(float);

		void* data;
		vkMapMemory(app->get_device(), hx->Memory, 0, imageSize, 0, &data);
		// Copy newData into data (assuming newData is of size `size`)

		float* pixels = reinterpret_cast<float*>(m_Displacements.data());

		memcpy(data, pixels, imageSize);
		vkUnmapMemory(app->get_device(), hx->Memory);
		
		auto& hd = get_image_fields()[1];

		VkDeviceSize imageSize2 = hd->Width * hd->Height * 4 * sizeof(float);

		void* data2;
		vkMapMemory(app->get_device(), hd->Memory, 0, imageSize2, 0, &data2);
		float* normals = reinterpret_cast<float*>(m_Normals.data());

		memcpy(data, normals, imageSize2);
		vkUnmapMemory(app->get_device(), hd->Memory);

	}

	int get_mandelbulb_factor() override { return this->mandelbulb_factor; }

	glm::vec3 fourier_amplitude(float k_len, std::mt19937& gen, std::uniform_real_distribution<float>& distribution, float theta, float delta_k, glm::vec2 k)
	{
		float g = 9.81f;
		float omega = pow(g * k_len, 0.5f); // dispertion relation

		float fetch = 200000.0f;
		float u_10 = 30.0f;

		float omega_p = 22 * pow(g * g / (u_10 * fetch), 0.333f); //angular frequency of the spectral peak

		float phi = 2*jonswap(omega, omega_p, fetch, u_10) * oceanography_donelan_banner_directional_spreading(omega, omega_p, glm::radians(45.0f)) * sqrt(g / k_len) / (2 * k_len) * delta_k * delta_k;
	//	float phi = phillips_spectrum(k.x, k.y);

		float factor = 1 / pow(2, 0.5f) * pow(phi, 0.5f);

		return glm::vec3(factor * distribution(gen), factor * distribution(gen), omega);
	}

	float __oceanography_hasselmann_s(float omega, float omega_peak, float u, float g)
	{
		float s0 = 6.97f * pow(omega / omega_peak, 4.06f);
		float s1_exp = -2.33f - 1.45f * (((u * omega_peak) / g) - 1.17f);
		float s1 = 9.77f * pow(omega / omega_peak, s1_exp);
		return omega > omega_peak
			? s1
			: s0;
	}

	float __oceanography_mitsuyasu_s(float omega, float omega_peak, float u, float g)
	{
		float s_p = 11.5f * pow((omega_peak * u) / g, -2.5f);
		float s0 = pow(s_p, 5.0f);
		float s1 = pow(s_p, -2.5f);
		return omega > omega_peak
			? s1
			: s0;
	}

	float math_stirling_approximation(float n)
	{
		return sqrt(2.0f * M_PI * n) * pow(n / glm::e<float>(), n);
	}

	float __oceanography_mitsuyasu_q(float s)
	{
		float a = pow(2.0f, 2.0f * s - 1) / M_PI;
		float b = pow(math_stirling_approximation(s + 1), 2.0f);
		float c = math_stirling_approximation(2.0f * s + 1);
		return a * (b / c);
	}

	float oceanography_mitsuyasu_directional_spreading(float omega, float omega_peak, float theta, float u, float g)
	{
		float s = __oceanography_mitsuyasu_s(omega, omega_peak, u, g);
		float q_s = __oceanography_mitsuyasu_q(s);
		return q_s * pow(abs(cos(theta / 2.0f)), 2.0f);
	}

	float oceanography_hasselmann_directional_spreading(float omega, float omega_peak, float theta, float u, float g)
	{
		float s = __oceanography_hasselmann_s(omega, omega_peak, u, g);
		float q_s = __oceanography_mitsuyasu_q(s);
		return q_s * pow(abs(cos(theta / 2.0f)), 2.0f);
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

	float jonswap(double  omega,float omega_p, float fetch, float U_10)
	{
		float g = 9.81f;

		float alpha = 0.076f * pow(pow(U_10, 2) / (fetch * g), 0.22f) * 1.3; // equilibrium range parameter

		float gamma = 3.3f;

		double sigma = omega < omega_p ? 0.07 : 0.09; // Standard deviation for JONSWAP spectrum


		float beta = 1.25f;
		float expr1 = pow(g, 2) / pow(omega, 5);
		float expr2 = exp(-beta * pow(omega_p / omega, 4));
		double S = alpha * expr1 * expr2;

		float r = exp(-pow(omega - omega_p, 2) / (2 * pow(sigma * omega_p, 2)));

		float peakEnhancementFactor = pow(gamma, r);

		float energy = S * peakEnhancementFactor;

		return energy;
	}

	float phillips_spectrum(float kx, float ky) {
		// Calculate wave vector length
		float k_squared = kx * kx + ky * ky;
		float k = sqrt(k_squared);

		glm::vec2 wind_direction = glm::vec2(0.0f, 1.0f);

		// Calculate wind speed magnitude

		// Calculate directional vector
		float k_dot_w = (kx / k) * (wind_direction.x) + (ky / k) * (wind_direction.y);

		float gravity = 9.81f;

		float wind_speed_mag = 10.0f;

		// Calculate exponent
		float L = wind_speed_mag * wind_speed_mag / gravity;
		float L_squared = L * L;
		float damping = 0.001f * L; // Damping factor to prevent division by zero
		float damping_squared = damping * damping;
		float exponent = -1.0f / (k_squared * L_squared) - k_squared * damping_squared;

		float A = 0.81f*0.7f / (512.f*512.f);

		// Calculate Phillips spectrum
		float P = A * exp(exponent) / (k_squared * k_squared) * k_dot_w * k_dot_w;

		return P;
	}



	std::random_device rd;
	std::mt19937 gen{ rd() };
	std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };



private:
	int mandelbulb_factor = 8;

};

