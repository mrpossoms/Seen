#include "seen.hpp"

int main(int arc, const char* argv[])
{
	seen::RendererGL renderer("./data", argv[0], 640, 480, 4, 0);
	seen::ListScene scene;
	seen::Camera camera(M_PI / 2, renderer.width, renderer.height);

	seen::Model* sky_sphere = seen::MeshFactory::get_model("sphere.obj");

	// define the sky render pass
	seen::CustomPass sky_pass([&](int index) {
		seen::ShaderProgram::builtin_sky().use();
		glDisable(GL_CULL_FACE);
	});
	seen::ListScene sky_scene = { sky_sphere };
	sky_pass.scene = &sky_scene;

	// callback for camera looking
	renderer.use_free_cam(camera);


	while(renderer.is_running())
	{
		renderer.draw(&camera, { &sky_pass });
	}

	return 0;
}
