#include "Layers/MyLayer.h"

#include <EntryPoint.h>

class MyApplication : public Application
{
public:
	MyApplication()
	{
		std::shared_ptr<Layer> layer = std::shared_ptr<Layer>(new MyLayer(64, 100.0f));
		push_layer(layer);
	}
};

std::unique_ptr<Application> CreateApplication()
{
	return std::unique_ptr<Application>(new MyApplication());
}

