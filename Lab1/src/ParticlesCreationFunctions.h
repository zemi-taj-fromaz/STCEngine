#pragma once

#include <functional>
#include <Engine.h>

namespace Functions
{
	extern std::function<std::vector<Particle>(void*)> mandelbulb;
	extern std::function<std::vector<Particle>(void*)> mandelbrot;
	extern std::function<std::vector<Particle>(void*)> fireworks;
};
