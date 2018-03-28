#include "seen.hpp"
#include "sky.hpp"
#include <png.h>
#include <sstream>

int main(int argc, const char* argv[])
{
	seen::RendererGL renderer("./data/", argv[0]);//, 256, 256);
	seen::ListScene scene;
	seen::Sky sky;
	seen::Camera camera(M_PI / 2, renderer.width, renderer.height);
	seen::Model* bale = seen::MeshFactory::get_model("mutable_cube.obj");
	seen::Material* bale_mat = seen::TextureFactory::get_material("hay");
	seen::Tex displacement_tex = seen::TextureFactory::load_texture("hay.displacement.png");
	seen::CustomPass bale_pass, sky_pass;
	Quat q_bale_ori;

	srand(time(NULL));

	seen::ShaderConfig bale_shader = {
		.vertex = "displacement.vsh",
		.fragment = "basic.fsh"
	};

	camera.position(0, 0, -3);

	// callback for camera looking
	renderer.mouse_moved = [&](double x, double y, double dx, double dy)
	{
		Quat pitch, yaw, roll;
		Vec3 forward, left, up;

		pitch.from_axis_angle(VEC3_LEFT.v[0], VEC3_LEFT.v[1], VEC3_LEFT.v[2], dy * 0.01);
		yaw.from_axis_angle(VEC3_UP.v[0], VEC3_UP.v[1], VEC3_UP.v[2], dx * 0.01);
		pitch = pitch * yaw;
		q_bale_ori = pitch * q_bale_ori;
	};

	float uv_rot = 0;
	bale_pass.preparation_function = [&]()
	{
		vec4_t material = { 0.1, 0.01, 1, 0.01 };
		vec4_t albedo = { 1, 1, 1, 1 };
		mat4x4_t world;
		mat3x3_t rot;
		vec3_t light_dir = { 1, 0, 1 };
		vec3 axis = { 0.0, 1.0, 0.0 };

		if (argc > 1)
		{
			light_dir.x = seen::rf(-1, 1);
			light_dir.z = seen::rf(-1, 1);
			uv_rot = seen::rf(0, 2 * M_PI);

			vec3_norm(axis, axis);
			quat_from_axis_angle(q_bale_ori.v, axis[0], axis[1], axis[2], seen::rf(0, 2 * M_PI));
			camera.fov(M_PI / (2 + (seen::rf() * 16)));
			mat4x4_translate(world.v, seen::rf(-1, 1), seen::rf(-1, 1), seen::rf(-1, 1));
		}
		else
		{
			uv_rot += 0.001f;
		}

		seen::ShaderProgram& shader = *seen::Shaders[bale_shader]->use();

		shader << bale_mat; //->use(&shader.draw_params.material_uniforms.tex);

		mat4x4_from_quat(world.v, q_bale_ori.v);

		for(int i = 3; i--;)
		for(int j = 3; j--;)
		{
			rot.v[i][j] = world.v[i][j];
		}

		shader["u_world_matrix"] << world;
		shader["u_normal_matrix"] << rot;

		shader["u_light_dir"] << light_dir;
		shader["u_texcoord_rotation"] << uv_rot;
		shader["u_green"] << seen::rf(0.5, 2);
		shader["us_displacement"] << displacement_tex;

	};

	// seen::ShaderProgram::active(seen::Shaders[sky_shader]);

	// Add all things to be drawn to the scene
	bale_pass.drawables = new std::vector<seen::Drawable*>();
	bale_pass.drawables->push_back(bale);

	scene.drawables().push_back(&bale_pass);

	int i = argc >= 3 ? atoi(argv[2]) : 10e6;
	for(; renderer.is_running() && i--;)
	{
		renderer.draw(&camera, &scene);

		if (argc >= 2)
		{
			std::stringstream path_ss;
			path_ss << argv[1] << "/" << std::hex << random();
			renderer.capture(path_ss.str());

			glClearColor(seen::rf(), seen::rf(), seen::rf(), 1);
		}
	}

	return 0;
}
