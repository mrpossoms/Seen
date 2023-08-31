#include "seen.hpp"

static float sphere(vec<3> v)
{
	return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) - 1.f;
}


int main(int argc, const char* argv[])
{
	seen::RendererGL renderer("./data", argv[0], 640, 480, 4, 0);
	seen::Camera cam(M_PI / 4, 640, 480);

	// setup camera
	cam.position(0, 1, 0);
	renderer.use_free_cam(cam);

	// Models
	seen::Model* sky_sphere = seen::MeshFactory::get_model("sphere.obj");
	seen::Model land(new seen::Heightmap("austrailia_256.png", 10, 256));
	auto dirt_mat = seen::TextureFactory::get_material("dirt");

	const float s = 4;
	auto mc_mesh = new seen::Volume(vec<3>(-s, -s, -s), vec<3>(s, s, s), 32);
	mc_mesh->generate(sphere);
	seen::Model monolith(mc_mesh);

	// setup the light
	float t = 0;
	seen::Light light;
	light.power = { 1.5, 1.5, 1.5 };
	light.ambience = 0.01;
	mat<4, 4>_perspective(light.projection.v, M_PI / 2, 1, 0.1, 100);

	// define render passes
	auto shadow_pass = seen::ShadowPass(512, true);

	seen::CustomPass sky_pass([&](int index) {
		seen::ShaderProgram::builtin_sky().use();
		glDisable(GL_CULL_FACE);
	});

	seen::CustomPass land_pass([&](int index) {
		light.position = { 2 * cos(t), 2, 2 * sin(t) };

		seen::ShaderProgram& shader = seen::ShaderProgram::builtin_realistic().use();

		shader << dirt_mat;
		shader << &shadow_pass;
		shader << &light;
	});

	// define scenes with their drawables
	auto land_scene = seen::ListScene{ &land, &monolith };
	auto sky_scene = seen::ListScene{ sky_sphere };

	// setup the shadow pass
	shadow_pass.scene = &land_scene;
	shadow_pass.lights = { &light };

	// assign scenes to their passes
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
