#pragma once
#define COMPUTE_JACOBIAN

#include "LayerInit.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

#include <fftw3.h>
#include <complex>
#include<map>

class MyLayer : public Layer
{
public:

	static constexpr uint32_t     s_kDefaultTileSize{ 256 };
	static constexpr float        s_kDefaultTileLength{ 500.0f };

	static inline const glm::vec2 s_kDefaultWindDir{ 1.0f, 1.0f };
	static constexpr float        s_kDefaultWindSpeed{ 50.0f };
	static constexpr float        s_kDefaultAnimPeriod{ 20.0f };
	static constexpr float        s_kDefaultPhillipsConst{ 50e-7f };
	static constexpr float        s_kDefaultPhillipsDamping{ 1.0f };

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
	float m_A{ 3.8e-6 };
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
	const float s_kOneOver2sqrt{ 1.0f / std::sqrt(2.0f) }; // 1/sqrt(2)

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
		//
		// var *= pow((kx * kx) / k_sq, wind_alignment);
		float cosFact = glm::dot(unitWaveVec, m_WindDir);
	//	float cosFact = (unitWaveVec.x * unitWaveVec.x) / k2;
		cosFact = cosFact * cosFact;



		const float L = m_WindSpeed * m_WindSpeed / s_kG;
		const float L2 = L * L;

		int wind_alignment = 2;

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

	auto  GetTileSize()  { return m_TileSize; }
	auto  GetTileLength()  { return m_TileLength; }
	auto  GetWindDir()  { return m_WindDir; }
	auto  GetWindSpeed()  { return m_WindSpeed; }
	auto  GetAnimationPeriod()  { return m_AnimationPeriod; }
	auto GetPhillipsConst()  { return m_A; }
	auto  GetDamping()  { return m_Damping; }
	auto  GetDisplacementLambda()  { return m_Lambda; }
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

	void Prepare()
	{
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
	}

	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------
	//---------------------------------------------------

	WaterSurfaceUBO m_Surface;
	SunPositionData m_SunPosition;

	MyLayer(uint32_t tileSize, float tileLength) : Layer("Example")
	{
		SetTileSize(tileSize);
		SetTileLength(tileLength);

		Prepare();

		m_Surface.absorpCoef = waterTypeCoeffsMap[keys[0]];
		m_Surface.scatterCoef = ComputeScatteringCoefPA01(scatterCoefs[0]);
		m_Surface.backscatterCoef = ComputeBackscatteringCoefPA01(m_Surface.scatterCoef);


		imguiEnabled = true;

		//------------------------------ DESCRIPTORS ---------------------------------------------------

		auto camera = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(CameraBufferObject), Functions::cameraUpdateFunc);
		auto objects = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(ObjectData) * 1000, Functions::objectsUpdateFunc);
		auto waves = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(WaveData) * 50, Functions::wavesUpdateFunc);

		auto resolution = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Resolution), Functions::resolutionUpdateFunc);
		auto amplitude = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Amplitude), Functions::amplitudeUpdateFunc);

		auto samplerVertex = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT);
		auto sampler = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL);

		auto image2dIn = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dIn2 = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dOut = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2dOut2 = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);
		auto image2DFragment = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL);

		auto totalTime = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);
		auto totalTimeCompute = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::totalTimeUpdateFunc);

		auto dispMap = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 3, sizeof(DisplacementData) * 70000, Functions::dispUpdateFunc);

		auto verticalFlag = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(bool), Functions::verticalFlagUpdateFunc);

		auto waterSurfaceUBO = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(WaterSurfaceUBO), Functions::surfaceUpdateFunc);

		auto mandelbulbFactor = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::mandelbulbFactorUpdateFunc);
		auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight), Functions::globalLightUpdateFunc);

		auto sunPosData = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SunPositionData), Functions::sunPositionUpdateFunc);

		
		create_descriptors({ dispMap,  sunPosData, camera, waterSurfaceUBO,amplitude, objects, resolution, totalTime, image2dOut2, image2DFragment });

		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------

		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;

		TopoloG imagefieldTopology({ camera, objects, waterSurfaceUBO, amplitude, totalTime, sunPosData, image2DFragment, image2dOut2 });
		TopoloG imagestoreTopology({ dispMap, image2DFragment, image2dOut2 });
		TopoloG skyboxTopology({ camera, sampler });
		TopoloG skyTopology({ objects, resolution, totalTime, sunPosData });

		auto imageFieldPipelineLayout = std::make_shared<PipelineLayout>(imagefieldTopology);
		auto imageStorePipelineLayout = std::make_shared<PipelineLayout>(imagestoreTopology);		
		auto skyboxPipelineLayout = std::make_shared<PipelineLayout>(skyboxTopology);
		auto skyPipelineLayout = std::make_shared<PipelineLayout>(skyTopology);

		create_layouts({ imageStorePipelineLayout,  imageFieldPipelineLayout, skyPipelineLayout });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------

		std::vector<std::string> imageFieldShaderNames({ "ImageFieldShader.vert", "ImageFieldShader.frag" });
		std::vector<std::string> imageStoreShaderNames({ "ImageStore.comp"});
		std::vector<std::string> skyboxShaderNames({ "SkyboxShader.vert", "SkyboxShader.frag" });
		std::vector<std::string> skyShaderNames({ "Sky.vert", "Sky.frag" });

		//------------------------------- PIPELINES ----------------------------------------------------------

		auto imagefieldPipeline = std::make_shared<Pipeline>(imageFieldPipelineLayout, imageFieldShaderNames, VK_FALSE, false, VK_CULL_MODE_NONE);
		auto imagestorePipeline = std::make_shared<Pipeline>(imageStorePipelineLayout, imageStoreShaderNames, VK_FALSE,false, VK_CULL_MODE_BACK_BIT);
		auto skyboxPipeline = std::make_shared<Pipeline>(skyboxPipelineLayout, skyboxShaderNames, VK_FALSE, true, VK_CULL_MODE_FRONT_BIT);
		auto skyPipeline = std::make_shared<Pipeline>(skyPipelineLayout, skyShaderNames, VK_FALSE, false, VK_CULL_MODE_NONE);


		create_pipelines({ imagefieldPipeline, skyPipeline, imagestorePipeline });
		m_ComputePipeline = imagestorePipeline;


		//------------------------- TEXTURES ---------------------------------------------------------------
		std::shared_ptr<Texture> skyboxTex = std::make_shared<Texture>("stormydays/");
		//create_textures({ skyboxTex });

		//-------------------------------------- IMAGE FIELDS --------------------------------------------

		std::shared_ptr<Texture> hx = std::make_shared<Texture>(tileSize, tileSize);
		std::shared_ptr<Texture> dh = std::make_shared<Texture>(tileSize, tileSize);

		create_image_fields({hx, dh});

		m_ComputePipeline->ImageFields.push_back(hx);
		m_ComputePipeline->ImageFields.push_back(dh);

		//----------------------- MESH --------------------------------------------------------------------
		Mesh box("skybox.obj");
		Mesh square("square.obj");
		Mesh cube(MeshType::Cube);
		Mesh quad(MeshType::Quad);
		
		Mesh plain(MeshType::Plain, tileSize, tileLength);

		//-------------------- WAVES ------------------------------------------------------------------------

		std::random_device rd;
		std::mt19937 gen(rd());

		//-------------------- LIGHTS -----------------------------------------------------------------------

		this->m_Camera = Camera();
		m_Camera.Position = glm::vec3(0.0f, 20.0f, 50.0f);

		//-------------------- MESH WRAPPERS ----------------------------------------------------------------

		std::vector<std::shared_ptr<MeshWrapper>> meshWrappers;

		auto reload = std::make_shared<MeshWrapper>(skyPipeline, quad);
		reload->scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f));
		//reload->translation = glm::translate(glm::mat4(1.0f), glm::vec3(-1.2f, 0.7f, 0.0f));

		meshWrappers.push_back(reload);

		for (int i = 0; i < 1; i++)
		{
			for (int j = 0; j < 1; j++)
			{
				auto woodbox = std::make_shared<MeshWrapper>(imagefieldPipeline, plain);
				woodbox->Billboard = false;
				woodbox->image_fields.push_back(hx);
				woodbox->image_fields.push_back(dh);
				woodbox->color = glm::vec4(0.2f, 0.2f, 0.8f, 1.0f);
				woodbox->translation = glm::translate(glm::mat4(1.0f), glm::vec3(i*tileLength,0.0f, j*tileLength));
				woodbox->scale = glm::scale(glm::mat4(1.0f), glm::vec3(3.0, 1.0f, 3.0f));
				meshWrappers.push_back(woodbox);
			}
		}


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
			const float kSigns[] = { 1.0f, 1.0f };

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
					m_Displacements[kIndex].w = jacobian;
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
			[A](auto& d) { d.y /= A; });

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

		float dt = app->get_delta_time();

		static float m_TimeCtr = 0.0f;

		m_TimeCtr += dt * m_AnimSpeed;

		app->set_amplitude(ComputeWaves(m_TimeCtr));

		app->set_surface(m_Surface);
		app->set_sun_pos_data(m_SunPosition);
		auto& hx = get_image_fields()[0];
		VkDeviceSize imageSize = hx->Width * hx->Height * 4 * sizeof(float);


		app->set_displacements(m_Displacements);
		app->set_normals(m_Normals);

		for (int i = 0; i < m_ComputePipeline->pipelineLayout->descriptorSetLayout.size(); ++i)
        {
            if (m_ComputePipeline->pipelineLayout->descriptorSetLayout[i]->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                    || m_ComputePipeline->pipelineLayout->descriptorSetLayout[i]->descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
                ) continue;
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline->pipelineLayout->layout, i, 1, &m_ComputePipeline->pipelineLayout->descriptorSetLayout[i]->descriptorSets[imageIndex], 0, nullptr);
        }


		for (int i = 0; i < m_ComputePipeline->ImageFields.size(); ++i)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline->pipelineLayout->layout, m_ComputePipeline->pipelineLayout->descriptorSetLayout.size() - m_ComputePipeline->ImageFields.size() + i, 1, &m_ComputePipeline->ImageFields[i]->descriptorSets[imageIndex], 0, nullptr);
		}

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline->pipeline);

		vkCmdDispatch(commandBuffer, 1, 512, 1);

		// Barrier to synchronize memory access between dispatches
		VkMemoryBarrier memoryBarrier2{};
		memoryBarrier2.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier2.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Access mask for writes in previous dispatch
		memoryBarrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;  // Access mask for reads in subsequent dispatch

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Source pipeline stage
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // Destination pipeline stage
			0,                                    // Dependency flags
			1,                                    // Memory barrier count
			&memoryBarrier2,                       // Pointer to memory barriers
			0,                                    // Buffer memory barrier count
			nullptr,                              // Pointer to buffer memory barriers
			0,                                    // Image memory barrier count
			nullptr                               // Pointer to image memory barriers
		);
	}

	float m_AnimSpeed{ 3.0 };
	using WaterTypeMap = std::map<std::string, glm::vec3>;

	WaterTypeMap waterTypeCoeffsMap = {
	{"I: Clearest open ocean", glm::vec3{0.420, 0.063, 0.019}},
	{"II: Clear open ocean", glm::vec3{0.465, 0.089, 0.068}},
	{"III: Turbid open ocean", glm::vec3{0.520, 0.120, 0.135}},
	{"1: Clearest coastal waters", glm::vec3{0.510, 0.120, 0.250}},
	{"3: Clear coastal waters", glm::vec3{0.560, 0.190, 0.390}},
	{"5: Semi-clear coastal waters", glm::vec3{0.650, 0.300, 0.560}},
	{"7: Turbid coastal waters", glm::vec3{0.780, 0.460, 0.890}},
	{"9: Most turbid coastal waters", glm::vec3{0.920, 0.630, 1.600}}
	};



	std::vector<std::string> getKeys(const WaterTypeMap& map) {
		std::vector<std::string> keys;
		keys.reserve(map.size());

		for (const auto& pair : map) {
			keys.push_back(pair.first);
		}

		return keys;
	}
	std::vector<std::string> keys = getKeys(waterTypeCoeffsMap);


	uint32_t m_WaterTypeCoefIndex{ 0 };
	uint32_t m_BaseScatterCoefIndex{ 0 };

	std::vector<float> scatterCoefs{ 0.037, 0.219, 1.824 };
	std::vector<std::string> scatterCoefStrings{ "I: Clear ocean", "1: Coastal ocean", "9: Turbid harbor" };

	static const inline glm::vec3 s_kWavelengthsRGB_nm{ 680, 550, 440 };

	static glm::vec3 ComputeScatteringCoefPA01(float b_lambda0)
	{
		return b_lambda0 * ((-0.00113 * s_kWavelengthsRGB_nm + 1.62517f)
			/
			(-0.00113 * 514.0 + 1.62517));
	}

	glm::vec3 ComputeBackscatteringCoefPigmentPA01(float C)
	{
		// Morel. Optical modeling of the upper ocean in relation to its biogenus
		//  matter content.
		const glm::vec3 b_w(0.0007, 0.00173, 0.005);

		// Ratio of backscattering and scattering coeffiecients of the pigments
		const glm::vec3 B_b = 0.002f + 0.02f * (0.5f - 0.25f *
			((1.0f / glm::log(10.0f)) * glm::log(C))
			) * (550.0f / s_kWavelengthsRGB_nm);
		// Scattering coefficient of the pigment
		const float b_p = 0.3f * glm::pow(C, 0.62f);

		return 0.5 * b_w + B_b * b_p;
	}

	glm::vec3 ComputeBackscatteringCoefPA01(const glm::vec3& b)
	{
		return 0.01829 * b + 0.00006f;
	}

	void set_callbacks(GLFWwindow* window) override
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
			auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
			app->set_frame_buffer_resized();
			//TODO
			//app->set_frame_b

			});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
			auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
			app->set_field_of_view(static_cast<float>(yoffset));
			//app->setScrollCallback();
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

	int get_mandelbulb_factor() override { return this->mandelbulb_factor; }

	std::random_device rd;
	std::mt19937 gen{ rd() };
	std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };

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

		if (ImGui::CollapsingHeader("Water Surface Settings",
			ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);

			ShowWaterSurfaceSettings(app);

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

	static glm::vec3 GetDirFromAngles(float inclination, float azimuth)
	{
		//inclination = glm::radians(inclination);
		//azimuth = glm::radians(azimuth);
		return glm::normalize(
			glm::vec3(glm::sin(inclination) * glm::cos(azimuth),
				glm::cos(inclination),
				glm::sin(inclination) * glm::sin(azimuth))
		);
	}

	void ShowLightingSettings(AppVulkanImpl* app)
	{
		static float sunAzimuth = 90.0f;        // [-pi, pi]
		static float sunInclination = 60.0f;    // [-pi/2, pi/2]
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.29f);

		auto& surface = app->get_surface();
		float fps = 1.0f / app->get_delta_time();

		ImGui::SliderFloat("FPS", &fps, 0.0f, 150.0f);

		ImGui::SliderFloat("Sky Intensity",
			&m_Surface.skyIntensity, 0.f, 10.f);
		ImGui::SliderFloat("Specular Intensity",
			&m_Surface.specularIntensity, 0.f, 3.f);
		ImGui::SliderFloat("Specular Highlights",
			& m_Surface.specularHighlights, 1.f, 64.f);

		// TODO change to wider range
		bool paramsChanged = ImGui::SliderAngle("##Sun Inclination",
			&sunInclination, -90.f, 90.f);
		ImGui::SameLine();
		paramsChanged |= ImGui::SliderAngle("Sun angles##Sun Azimuth",
			&sunAzimuth, -180.f, 180.f);

		if (paramsChanged)
		{
			m_SunPosition.Direction = GetDirFromAngles(sunInclination, sunAzimuth);
		}
		ImGui::PopItemWidth();

		ShowComboBox("Absorption type",
			keys.data(),
			keys.size(),
			keys[m_WaterTypeCoefIndex].c_str(),
			&m_WaterTypeCoefIndex);

		m_Surface.absorpCoef = waterTypeCoeffsMap[keys[m_WaterTypeCoefIndex]];
		ShowComboBox("Scattering type",
			scatterCoefStrings.data(),
			scatterCoefStrings.size(),
			scatterCoefStrings[m_BaseScatterCoefIndex].c_str(),
			&m_BaseScatterCoefIndex);

		m_Surface.scatterCoef =
			ComputeScatteringCoefPA01(
				scatterCoefs[m_BaseScatterCoefIndex]);


		static bool usePigment = false;
		ImGui::Checkbox(" Consider pigment concentration", &usePigment);
		if (usePigment)
		{
			static float pigmentC = 1.0;
			ImGui::SliderFloat("Pigment concentration", &pigmentC, 0.001f, 3.f);

			m_Surface.backscatterCoef =
				ComputeBackscatteringCoefPigmentPA01(pigmentC);
		}
		else
		{
			m_Surface.backscatterCoef =
				ComputeBackscatteringCoefPA01(m_Surface.scatterCoef);
		}

		//// Terrain
		ImGui::ColorEdit3("Seabed Base Color",
			glm::value_ptr(surface.terrainColor));
		//ImGui::DragFloat("Rest Ocean Level", &surface.height,
		//	1.0f, 0.0f, 1000.0f);
	//	ImGui::Checkbox(" Clamp to wave height", &m_ClampHeight);
	}

	void ShowWaterSurfaceSettings(AppVulkanImpl* app)
	{

		static glm::vec2 windDir = GetWindDir();
		static float windSpeed = GetWindSpeed();
		static float animPeriod = GetAnimationPeriod();
		static float phillipsA = GetPhillipsConst() * 1e7;
		static float damping = GetDamping();
		static float lambda = GetDisplacementLambda();

		// TODO unit
		// TODO 2 angles
		//ImGui::DragFloat2("Wind Direction", glm::value_ptr(windDir),
		//	0.1f, 0.0f, 0.0f, "%.1f");
		//ImGui::DragFloat("Wind Speed", &windSpeed, 0.1f);
		//ImGui::DragFloat("Choppy correction", &lambda,
		//	0.1f, -8.0f, 8.0f, "%.1f");
		//ImGui::DragFloat("Animation Period", &animPeriod, 1.0f, 1.0f, 0.0f, "%.0f");
		//ImGui::DragFloat("Animation speed", &m_AnimSpeed, 0.1f, 0.1f, 8.0f);

		if (ImGui::TreeNodeEx("Water Properties and Lighting", ImGuiTreeNodeFlags_DefaultOpen))
		//	 ImGuiTreeNodeFlags_DefaultOpen)
		{
			ShowLightingSettings(app);
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Phillips Spectrum", ImGuiTreeNodeFlags_DefaultOpen))
			//, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat("Amplitude (10^-7)", &phillipsA, 0.1f, 1.0f, 10.0f,
				"%.2f");
		//	std::cout << phillipsA << std::endl;
			ImGui::DragFloat("Damping factor", &damping, 0.0001f, 0.0f, 1.0f,
				"%.4f");
			ImGui::TreePop();
		}

		if (ImGui::Button("Apply"))
		{
			const bool kWindDirChanged =
				glm::any(glm::epsilonNotEqual(windDir, GetWindDir(),
					glm::vec2(0.001f)));
			const bool kWindSpeedChanged =
				glm::epsilonNotEqual(windSpeed, GetWindSpeed(),
					0.001f);
			const bool kAnimationPeriodChanged =
				glm::epsilonNotEqual(animPeriod, GetAnimationPeriod(), 0.001f);
			const bool kPhillipsConstChanged =
				glm::epsilonNotEqual(phillipsA * 1e-7f, GetPhillipsConst(), 1e-8f);
			const bool kDampingChanged =
				glm::epsilonNotEqual(damping, GetDamping(),
					0.001f);

			const bool kNeedsPrepare =
				kWindDirChanged ||
				kWindSpeedChanged ||
				kAnimationPeriodChanged ||
				kPhillipsConstChanged ||
				kDampingChanged;


			if (kNeedsPrepare)
			{
				SetWindDirection(windDir);
				SetWindSpeed(windSpeed);
				SetAnimationPeriod(animPeriod);
				SetPhillipsConst(phillipsA * 1e-7);
				SetDamping(damping);

				Prepare();
				//	m_FrameMapNeedsUpdate = true;
			}

			SetLambda(lambda);
		}
	}

	static void ShowComboBox(const char* name,
		std::string* items, const uint32_t kItemCount,
		const char* previewValue, uint32_t* pCurrentIndex)
	{
		if (ImGui::BeginCombo(name, previewValue))
		{
			for (uint32_t n = 0; n < kItemCount; ++n)
			{
				const bool is_selected = (*pCurrentIndex == n);
				if (ImGui::Selectable(items[n].c_str(), is_selected))
					*pCurrentIndex = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}


private:
	int mandelbulb_factor = 8;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
};

