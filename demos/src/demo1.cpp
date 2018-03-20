#include "seen.hpp"
#include "sky.hpp"

int main(int arc, const char* argv[])
{
	seen::RendererGL renderer("../data", argv[0]);
	seen::ListScene scene;
	seen::Sky sky;
	seen::Camera camera(renderer.width, renderer.height, M_PI / 2);

	seen::ShaderConfig default_shader = {
		.vertex = "sky.vsh",
		.fragment = "sky.fsh"
	};

	mat4x4 view;

	mat4x4_look_at(view, VEC3_ZERO.v, VEC3_FORWARD.v, VEC3_UP.v);
	camera.view(view);

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

	seen::ShaderProgram::active(seen::Shaders[default_shader]);

	scene.drawables().push_back(&sky);

	while(renderer.is_running())
	{
		renderer.draw(&camera, &scene);
	}

	return 0;
}
