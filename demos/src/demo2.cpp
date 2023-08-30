#include "seen.hpp"
#include "sky.hpp"
#include <png.h>
#include <sstream>

int main(int argc, const char* argv[])
{
	seen::RendererGL renderer("./data/", argv[0], 256, 256, 4, 0);//, 256, 256);
	seen::ListScene scene;
	seen::Camera camera(M_PI / 2, renderer.width, renderer.height);
	seen::Model* bale = seen::MeshFactory::get_model(std::string(argv[1]) + ".obj");
	seen::Material* bale_mat = seen::TextureFactory::get_material(argv[1]);

	std::string disp_name = std::string(argv[1]) + ".displacement.png";

	seen::Tex displacement_tex = seen::TextureFactory::load_texture(disp_name);
	seen::CustomPass bale_pass, bale_tess_pass;
	quat q_bale_ori;

	srand(time(NULL));

	seen::ShaderConfig bale_shader = {
		.vertex = "displacement.vsh",
		.tessalation = {
			.control = "displacement.tcs",
			.evaluation = "displacement.tes",
		},
		.geometry = "",
		.fragment = "basic.fsh"
	};

	camera.position(0, 0, -3);

	// callback for camera looking
	renderer.use_free_cam(camera);

	float uv_rot = 0;
	bale_pass.preparation_function = [&](int instance_index)
	{
		vec4_t material = { 0.1, 0.01, 1, 0.01 };
		vec4_t albedo = { 1, 1, 1, 1 };
		mat<4,4> world;
		mat<3,3> rot;
		vec3_t light_dir = { 1, 0, 1 };
		vec3 axis = { 0.0, 1.0, 0.0 };

		if (argc > 2)
		{
			light_dir.x = seen::rf(-1, 1);
			light_dir.z = seen::rf(-1, 1);
			uv_rot = seen::rf(0, 2 * M_PI);

			vec3_norm(axis, axis);
			quat_from_axis_angle(q_bale_ori.v, axis[0], axis[1], axis[2], seen::rf(0, 2 * M_PI));
			camera.fov(M_PI / seen::rf(1,3));
			mat<4,4>ranslate(world.v, seen::rf(-0.5, 0.5), seen::rf(-1, 1), seen::rf(-1, 1));
		}
		else
		{
			uv_rot += 0.0001f;
		}

		seen::ShaderProgram& shader = seen::Shaders[bale_shader]->use();

		shader << bale_mat; //->use(&shader.draw_params.material_uniforms.tex);

		mat<4, 4>_from_quat(world.v, q_bale_ori.v);

		for(int i = 3; i--;)
		for(int j = 3; j--;)
		{
			rot.v[i][j] = world.v[i][j];
		}

		shader["u_world_matrix"] << world;
		shader["u_normal_matrix"] << rot;

		vec3_t tint = { 0.75, 0.75, 0.75 };

		shader["u_light_dir"] << light_dir;
		shader["u_texcoord_rotation"] << uv_rot;
		shader["u_displacement_weight"] << 0.0f;
		shader["TessLevelInner"] << 1.0f;
		shader["TessLevelOuter"] << 1.0f;
		shader["u_tint"] << tint;
		// glDisable(GL_CULL_FACE);
	};

	bale_tess_pass.preparation_function = [&](int instance_index)
	{
		seen::ShaderProgram& shader = seen::Shaders[bale_shader]->use();

		shader["us_displacement"] << displacement_tex;

		vec3_t tint = { 1.0, 1.0, 1.0 };

		shader["u_displacement_weight"] << 0.5f;
		shader["TessLevelInner"] << 5.0f;
		shader["TessLevelOuter"] << 13.0f;
		shader["u_tint"] << tint;
	};

	seen::ListScene bale_scene = { bale };
	bale_pass.scene      = &bale_scene;
	bale_tess_pass.scene = &bale_scene;

	renderer.clear_color(seen::rf(), seen::rf(), seen::rf(), 1);

	int i = argc >= 3 ? atoi(argv[3]) : 10e6;
	for(; renderer.is_running() && i--;)
	{
		renderer.draw(&camera, { &bale_pass, &bale_tess_pass });

		if (argc > 2)
		{
			std::stringstream path_ss;
			path_ss << argv[2] << "/" << std::hex << random();
			renderer.capture(path_ss.str());

			renderer.clear_color(seen::rf(), seen::rf(), seen::rf(), 1);
		}
	}

	return 0;
}
