#include "Layers/MyLayer.h"

#include <EntryPoint.h>

class MyApplication : public Application
{
public:
	MyApplication()
	{
		std::shared_ptr<Layer> layer = std::shared_ptr<Layer>(new MyLayer(256, 1024.0f, 11.0f));
		//drugi faktor je velicina polja
		//treci faktor je proprcionalan sa uzburkanoscu mora
		push_layer(layer);
	}
};

std::unique_ptr<Application> CreateApplication()
{
	return std::unique_ptr<Application>(new MyApplication());
}

