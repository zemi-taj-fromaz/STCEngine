#include <functional>
#include <Engine.h>

namespace Functions
{

	glm::vec3 cartesianToPolar(glm::vec3& cartesian)
	{
		float r = length(cartesian);

		// Avoid division by zero
		float theta;
		if (r > 0.0f)
		{
			theta = acos(cartesian.z / r);
		}
		else
		{
			theta = 0.0f;
		}

		float phi = atan2(cartesian.y, cartesian.x);

		return { r, theta, phi };
	}
	glm::vec3 polarToCartesian(glm::vec3& polar)
	{
		float x = polar.x * sin(polar.y) * cos(polar.z);
		float y = polar.x * sin(polar.y) * sin(polar.z);
		float z = polar.x * cos(polar.y);

		return { x , y, z };
	};

	std::function<std::vector<Particle>(void*)> mandelbulb = [](void* n)
	{
		std::vector<Particle> particles;
		int DIM = 64;

		const double MIN_X = -1.0;
		const double MAX_X = 1.0;
		const double MIN_Y = -1.0;
		const double MAX_Y = 1.0;
		const double MIN_Z = -1.0;
		const double MAX_Z = 1.0;

		glm::vec3 startColor(0.0f, 0.0f, 0.0f); // Starting color (e.g., black)
		glm::vec3 endColor(1.0f, 1.0f, 1.0f);

		float r, theta, phi;
		int step = 2;
		int divergence_count = 0;

		int from = -DIM / 2;
		int to = DIM / 2;
		int white_count = 0;
		for (int i = 0; i < DIM; i += step)
		{
			for (int j = 0; j < DIM; j += step)
			{
				for (int k = 0; k < DIM; k += step * 0.5)
				{
					float x = MIN_X + (MAX_X - MIN_X) * static_cast<double>(i) / DIM;
					float y = MIN_Y + (MAX_Y - MIN_Y) * static_cast<double>(j) / DIM;
					float z = MIN_Z + (MAX_Z - MIN_Z) * static_cast<double>(k) / DIM;

					glm::vec3 C{ x, y, z };

					int maxIterations = 100;

					int p;
					glm::vec3 Z(0);
					for (p = 0; p < maxIterations; ++p)
					{
						// Mandelbulb formula
						glm::vec3 polarZ = cartesianToPolar(Z);

						float power = 8.0; // Power parameter, adjust as needed

						polarZ.x = pow(polarZ.x, power);
						polarZ.y *= power;
						polarZ.z *= power;

						Z = polarToCartesian(polarZ) + C;

						// Escape condition
						if (length(Z) > 4) {
							divergence_count++;
							break;
						}
					}

					float t = static_cast<float>(p) / static_cast<float>(maxIterations);

					if (t == 1)
					{
						white_count++;
					}
					else
					{
						continue;
					}

					Particle Particle;
					Particle.position = glm::vec3(i, j, k);
					//Particle.color = glm::vec4(glm::mix(startColor, endColor, t), 1.0f);
					Particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

					particles.push_back(Particle);

				}
			}
		}

		return particles;
	};

	std::function<std::vector<Particle>(void*)> mandelbrot = [](void* n)
	{
		std::vector<Particle> particles;

		auto windowDims = reinterpret_cast<WindowDims*>(n);
		int width = windowDims->W;
		int height = windowDims->H;

		const double MIN_X = -2.5;
		const double MAX_X = 1.5;
		const double MIN_Y = -1.5;
		const double MAX_Y = 1.5;

		glm::vec3 startColor(0.0f, 0.0f, 0.0f); // Starting color (e.g., black)
		glm::vec3 endColor(1.0f, 1.0f, 1.0f);

		int step = 3;
		float aspectRatio = static_cast<float>(width) / height;

		for (int i = 0; i < width; i += step * aspectRatio)
		{
			for (int j = 0; j < height; j += step)
			{
				float x = MIN_X + (MAX_X - MIN_X) * static_cast<double>(i) / width;
				float y = MIN_Y + (MAX_Y - MIN_Y) * static_cast<double>(j) / height;

				glm::vec2 C{ x, y };

				int maxIterations = 100;

				int p;
				int iter = 0;
				// glm::vec3 Z = cartesian;
				glm::vec3 Z = { 0.0f, 0.0f, 0.0f };
				for (p = 0; p < maxIterations; ++p)
				{
					// Mandelbrot formula
					glm::vec2 Z2 = glm::vec2(Z.x, Z.y);
					glm::vec2 Ztemp = Z2;

					Z2.x = Ztemp.x * Ztemp.x - Ztemp.y * Ztemp.y + C.x;
					Z2.y = 2.0 * Ztemp.x * Ztemp.y + C.y;

					Z = glm::vec3(Z2.x, Z2.y, 0.0);

					// Escape condition
					if (length(Z) > 2.0) {
						break;
					}
				}

				float t = static_cast<float>(p) / static_cast<float>(maxIterations);


				Particle particle;
				particle.position = glm::vec3((2.0f * i) / width - 1.0f, (1.0f - (2.0f * j) / height), 0.0f);
				particle.position.x *= aspectRatio;

				particle.color = glm::vec4(glm::mix(startColor, endColor, t), 1.0f);

				particles.push_back(particle);
			}
		}

		return particles;
	};

	std::function<std::vector<Particle>(void*)> fireworks = [](void* n)
	{
		auto windowDims = reinterpret_cast<WindowDims*>(n);
		int width = windowDims->W;
		int height = windowDims->H;

		std::default_random_engine rndEngine((unsigned)time(nullptr));
		std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);
		std::vector<Particle> particles(200);
		for (auto& particle : particles) {
			float r = 0.25f * sqrt(rndDist(rndEngine));
			float theta = rndDist(rndEngine) * 2 * 3.14159265358979323846;
			float x = r * cos(theta) * height / width;
			float y = r * sin(theta);

			particle.position = glm::vec3(x, y, 0);
			//  particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.25f;
			particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine), 1.0f);
		}

		return particles;
	};
}