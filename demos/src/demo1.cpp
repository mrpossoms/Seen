#include "seen.hpp"

int main(int arc, const char* argv[])
{
	seen::RendererGL renderer("./data", argv[0], 640, 480, 4, 0);
	seen::ListScene scene;
	seen::Camera camera(M_PI / 2, renderer.width, renderer.height);

	seen::Model* sky_sphere = seen::MeshFactory::get_model("sphere.obj");

	// define the sky render pass 
	seen::CustomPass sky_pass([&](int index) {
		seen::ShaderProgram::builtin_sky()->use();
		glDisable(GL_CULL_FACE);
	}, NULL);
	seen::ListScene sky_scene;
	
	sky_scene.drawables().push_back(sky_sphere);
	sky_pass.scene = &sky_scene;

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
	seen::ShaderProgram::builtin_sky()->use();

	while(renderer.is_running())
	{
		renderer.draw(&camera, { &sky_pass });
	}

	return 0;
}
