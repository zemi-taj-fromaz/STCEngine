#include <Engine.h>
#include <optional>

class ExampleLayer : public Layer
{
public:
	ExampleLayer() : Layer("Example") 
	{
		create_descriptors(
			{
				
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
				//{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
			}
			);
		
		create_layouts(
			{
				{{0, 1, 2, 3}},
				{{0, 3}},
				{{0, 1, 2}},
				{{0, 2, 3}},
			}
			);

		create_pipelines(
			{
				{0, "TextureShader.vert", "TextureShader.frag"},
				{2, "PlainShader.vert", "PlainShader.frag"},
				{2, "IlluminateShader.vert", "IlluminateShader.frag"},
				{1, "SkyboxShader.vert", "SkyboxShader.frag", VK_FALSE, true, VK_CULL_MODE_FRONT_BIT},
				{3, "ParticleShader.vert", "ParticleShader.frag", VK_TRUE, false, VK_CULL_MODE_NONE },
			}
			);

		create_textures(
			{
				{"stormydays/"},
				{"BODYMAINCOLORCG.png"},
				{"smoke.bmp"},
			}
		);

		create_mesh(
			{
			//	{2, {"cat.obj"}, true, std::nullopt, "spiral.txt"},
				//{1, {"panda.obj"}, false},
				{0, {"fighter_jet.obj"}, false, 1, "spiral.txt"},
				{3, {"skybox.obj"} , false, 0, std::nullopt, true},
			}
		);
		create_particles(
			{
				//{150, {4, {"texture.obj"}, false, 2}},
				{150, {4, {"texture - Copy.obj"}, false, 2}}
			}
		);

	}

private:
	
};


class MyApplication : public Application
{
public:
	MyApplication()
	{
		std::shared_ptr<Layer> layer = std::shared_ptr<Layer>(new ExampleLayer());
		push_layer(layer);
	}
};

std::unique_ptr<Application> CreateApplication()
{
	return std::unique_ptr<Application>(new MyApplication());
}

