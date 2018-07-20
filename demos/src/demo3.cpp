#include "seen.hpp"

seen::ShaderProgram* land_shader;

void setup_shaders()
{
	auto vsh = seen::Shader::vertex("basic_vsh");
	vsh.vertex(seen::Shader::VERT_POSITION | seen::Shader::VERT_NORMAL | seen::Shader::VERT_TANGENT | seen::Shader::VERT_UV)
	   .transformed()
   	   .compute_binormal()
	   .viewed().projected().pass_through("texcoord_in")
	   .emit_position()
	   .next(vsh.builtin("gl_Position") = vsh.local("l_pos_proj"));

	auto fsh = seen::Shader::fragment("basic_fsh");
	fsh.preceded_by(vsh);
	fsh.color_textured()
	   .normal_mapped()
	   .shadow_mapped_vsm()
	//    .shadow_mapped()
	   .blinn()
	   ;

	std::cerr << fsh.code() << '\n';

	land_shader = seen::ShaderProgram::compile({ vsh, fsh });
	land_shader->use();
}


int main(int argc, const char* argv[])
{
	seen::RendererGL renderer("./data", argv[0], 640, 480, 4, 0);
	seen::Camera cam(M_PI / 4, 640, 480);

	setup_shaders();

	auto height_map = seen::TextureFactory::load_texture("austrailia_256.png");
	auto normal_tex = seen::TextureFactory::load_texture("dirt_normal.png");
	auto dirt_tex = seen::TextureFactory::load_texture("dry_dirt.png");

	// setup camera
	cam.position(0, 1, 0);
	renderer.use_free_cam(cam);

	// Models
	seen::Model* sky_sphere = seen::MeshFactory::get_model("sphere.obj");
	seen::Model land(new seen::Heightmap(height_map, 10, 256));

	seen::CustomPass sky_pass([&](int index) {
		seen::ShaderProgram::builtin_sky()->use();
		glDisable(GL_CULL_FACE);
	}, NULL);

	float t = 0;

	auto shadow_pass = seen::ShadowPass(1024);

	seen::Light light;
	light.power = { 1, 1, 1 };
	light.is_point = true;
	light.ambience = 0.01;

	seen::CustomPass land_pass([&](int index) {
		light.position.x = 3 * cos(t);
		light.position.y = 1;
		light.position.z = 3 * sin(t);

		glEnable(GL_CULL_FACE);

		land_shader->use();

		(*land_shader)["u_color_sampler"] << dirt_tex;
		(*land_shader)["u_normal_sampler"] << normal_tex;
		(*land_shader)["u_view_position"] << cam.position();
		(*land_shader) << &shadow_pass;
		(*land_shader) << &light;
	}, NULL);

	seen::ListScene land_scene, sky_scene;

	sky_scene.drawables().push_back(sky_sphere);
	land_scene.drawables().push_back(&land);


	shadow_pass.scene = &land_scene;
	shadow_pass.lights.push_back(&light);

	sky_pass.scene = &sky_scene;
	land_pass.scene = &land_scene;

	while (renderer.is_running())
	{
		t += 0.01;

		renderer.draw(&cam, {
			&shadow_pass,
			&sky_pass,
			&land_pass
		});
	}

	return 0;
}
//
