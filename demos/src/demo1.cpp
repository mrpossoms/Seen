#include "seen.hpp"
#include "sky.hpp"

int main(int arc, const char* argv[])
{
	seen::RendererGL renderer("./data", argv[0]);
	seen::ListScene scene;
	seen::Sky sky;
	seen::Camera camera(M_PI / 2, renderer.width, renderer.height);

	seen::Mesh* bale = seen::MeshFactory::get_model("mutable_cube.obj");

	// define the sky shader
	seen::ShaderConfig default_shader = {
		.vertex = "sky.vsh",
		.fragment = "sky.fsh"
	};

	// callback for camera looking
	renderer.mouse_moved = [&](double x, double y, double dx, double dy)
	{
		Quat q = camera.orientation();
		Quat pitch, yaw, roll;
		Vec3 forward, left, up;

		pitch.from_axis_angle(VEC3_LEFT.v[0], VEC3_LEFT.v[1], VEC3_LEFT.v[2], dy * 0.01);
		yaw.from_axis_angle(VEC3_UP.v[0], VEC3_UP.v[1], VEC3_UP.v[2], dx * 0.01);
		pitch = pitch * yaw;
		q = pitch * q;

		camera.orientation(q);
	};

	// set the sky shader to active
	seen::ShaderProgram::active(seen::Shaders[default_shader]);

	// Add all things to be drawn to the scene
	scene.drawables().push_back(&sky);

	while(renderer.is_running())
	{
		renderer.draw(&camera, &scene);
	}

	return 0;
}
