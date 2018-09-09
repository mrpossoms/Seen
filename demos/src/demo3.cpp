#include "seen.hpp"

static float sphere(vec3 v)
{
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) - 1.f;
}


int main(int argc, const char* argv[])
{
	seen::RendererGL renderer("./data", argv[0], 640, 480, 4, 0);
	seen::Camera cam(M_PI / 4, 640, 480);

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
	   .blinn()
	   ;

	//land_shader = seen::ShaderProgram::builtin_normal_colors();
	seen::ShaderProgram::compile("land", { vsh, fsh }).use();

	// setup camera
	cam.position(0, 1, 0);
	renderer.use_free_cam(cam);

	// Models
	seen::Model* sky_sphere = seen::MeshFactory::get_model("sphere.obj");
	seen::Model land(new seen::Heightmap("austrailia_256.png", 10, 256));
	auto dirt_mat = seen::TextureFactory::get_material("dirt");

	const float s = 4;
	auto mc_mesh = new seen::Volume(Vec3(-s, -s, -s), Vec3(s, s, s), 64);
	mc_mesh->generate(sphere);
	seen::Model monolith(mc_mesh);

	float t = 0;
	seen::Light light;
	light.power = { 1.5, 1.5, 1.5 };
	light.ambience = 0.01;
	mat4x4_perspective(light.projection.v, M_PI / 2, 1, 0.1, 100);

	// render passes
	seen::CustomPass sky_pass([&](int index) {
		seen::ShaderProgram::builtin_sky().use();
		glDisable(GL_CULL_FACE);
	});

	auto shadow_pass = seen::ShadowPass(512, true);

	seen::CustomPass land_pass([&](int index) {
		light.position = { 2 * cos(t), 2, 2 * sin(t) };

		seen::ShaderProgram& shader = seen::ShaderProgram::get("land").use();

		shader << dirt_mat;
		shader["u_view_position"] << cam.position();
		shader << &shadow_pass;
		shader << &light;
	});

	seen::ListScene land_scene = { &land, &monolith };
	seen::ListScene sky_scene = { sky_sphere };

	shadow_pass.scene = &land_scene;
	shadow_pass.lights = { &light };

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
