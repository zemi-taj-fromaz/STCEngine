#include "Layers/EndGameLayer.h"
#include "Layers/GameLayer.h"

#include <EntryPoint.h>

class MyApplication : public Application
{
public:
	MyApplication()
	{
		std::shared_ptr<Layer> layer = std::shared_ptr<Layer>(new GameLayer());
		std::shared_ptr<Layer> endlayer = std::shared_ptr<Layer>(new EndGameLayer());
		push_layer(layer);
		push_layer(endlayer);
	}
};

std::unique_ptr<Application> CreateApplication()
{
	return std::unique_ptr<Application>(new MyApplication());
}

