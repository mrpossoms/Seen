#include "seen.hpp"
#include "sky.hpp"

int main(int arc, const char* argv[])
{
	seen::RendererGL renderer("../data", argv[0]);
	seen::ListScene scene;
	seen::Sky sky;
	seen::Camera camera(renderer.width, render.height, M_PI / 2);

	seen::ShaderConfig default_shader = {
		.vertex = "basic.vsh",
		.fragment = "basic.fsh"
	};

	mat4x4 view;

	mat4x4_look_at(view, VEC3_ZERO.v, VEC3_FORWARD.v, VEC3_UP.v);
	camera.view(view);

	seen::ShaderProgram::active(seen::Shaders[default_shader]);

	scene.drawables().push_back(&sky);

	while(renderer.is_running())
	{
		renderer.draw(NULL, &scene);
	}

	return 0;
}
