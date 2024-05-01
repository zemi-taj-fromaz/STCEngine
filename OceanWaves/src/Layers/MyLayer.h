#pragma once

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

		float cosFact = glm::dot(unitWaveVec, m_WindDir);
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


	MyLayer(uint32_t tileSize, float tileLength) : Layer("Example")
	{
		SetTileSize(tileSize);
		SetTileLength(tileLength);

		Prepare();

		imguiEnabled = false;

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

		auto verticalFlag = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(bool), Functions::verticalFlagUpdateFunc);


		auto waterSurfaceUBO = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(WaterSurfaceUBO), Functions::surfaceUpdateFunc);

		auto mandelbulbFactor = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(float), Functions::mandelbulbFactorUpdateFunc);
		auto globalLight = std::make_shared<Descriptor>(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalLight), Functions::globalLightUpdateFunc);

		create_descriptors({ verticalFlag, camera, waterSurfaceUBO,amplitude, objects, resolution, totalTime, mandelbulbFactor, globalLight, waves,sampler, image2dIn, image2dIn2, image2dOut, image2dOut2, image2DFragment, totalTimeCompute });

		//------------------------------ PIPELINE LAYOUTS ---------------------------------------------------

		using TopoloG = std::vector<std::shared_ptr<Descriptor>>;

		TopoloG imagefieldTopology({ camera, objects, waterSurfaceUBO, amplitude, image2DFragment, image2dOut2 });


		auto imageFieldPipelineLayout = std::make_shared<PipelineLayout>(imagefieldTopology);
		

		create_layouts({  imageFieldPipelineLayout });// , pipelineLayoutCompute, pipelineLayoutGraphics

		//------------------------------- SHADERS ------------------------------------------------------------------


		std::vector<std::string> imageFieldShaderNames({ "ImageFieldShader.vert", "ImageFieldShader.frag" });

		//------------------------------- PIPELINES ----------------------------------------------------------

		auto imagefieldPipeline = std::make_shared<Pipeline>(imageFieldPipelineLayout, imageFieldShaderNames, VK_TRUE, false, VK_CULL_MODE_NONE);



		create_pipelines({ imagefieldPipeline });

		//------------------------- TEXTURES ---------------------------------------------------------------


		//-------------------------------------- IMAGE FIELDS --------------------------------------------

		std::shared_ptr<Texture> hx = std::make_shared<Texture>(tileSize, tileSize);
		std::shared_ptr<Texture> dh = std::make_shared<Texture>(tileSize, tileSize);
		//waveHeightField->DescriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;


		create_image_fields({hx, dh });


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

		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				auto woodbox = std::make_shared<MeshWrapper>(imagefieldPipeline, plain);
				woodbox->Billboard = false;
				woodbox->image_fields.push_back(hx);
				woodbox->image_fields.push_back(dh);
				woodbox->color = glm::vec4(0.2f, 0.2f, 0.8f, 1.0f);
				woodbox->translation = glm::translate(glm::mat4(1.0f), glm::vec3(i*tileLength,0.0f, j*tileLength));
				meshWrappers.push_back(woodbox);

			}
		}


	//	meshWrappers.push_back(heightmap);

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

	//	return NormalizeHeights(masterMinHeight, masterMaxHeight);
		return 1.0f;
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
		
		float dt = app->get_delta_time();

		static float m_TimeCtr = 0.0f;

		m_TimeCtr += dt * m_AnimSpeed;

		app->set_amplitude(ComputeWaves(m_TimeCtr));
		
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

		memcpy(data2, normals, imageSize2);
		vkUnmapMemory(app->get_device(), hd->Memory);

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


	int get_mandelbulb_factor() override { return this->mandelbulb_factor; }

	std::random_device rd;
	std::mt19937 gen{ rd() };
	std::uniform_real_distribution<float> distribution{ 0.0f, 1.0f };



private:
	int mandelbulb_factor = 8;

};

