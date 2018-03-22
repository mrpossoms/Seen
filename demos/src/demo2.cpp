#include "seen.hpp"
#include "sky.hpp"

int main(int arc, const char* argv[])
{
	seen::RendererGL renderer("./data/", argv[0]);
	seen::ListScene scene;
	seen::Sky sky;
	seen::Camera camera(M_PI / 2, renderer.width, renderer.height);
	seen::Model* bale = seen::MeshFactory::get_model("mutable_cube.obj");
	seen::Material* bale_mat = seen::TextureFactory::get_material("hay");
	seen::CustomPass bale_pass, sky_pass;

	// define the sky shader
	seen::ShaderConfig sky_shader = {
		.vertex = "sky.vsh",
		.fragment = "sky.fsh"
	};

	seen::ShaderConfig bale_shader = {
		.vertex = "displacement.vsh",
		.fragment = "global.fsh"
	};

	camera.position(0, 0, -3);

	// callback for camera looking
	Quat q_bale_ori;

	renderer.mouse_moved = [&](double x, double y, double dx, double dy)
	{
		Quat pitch, yaw, roll;
		Vec3 forward, left, up;

		pitch.from_axis_angle(VEC3_LEFT.v[0], VEC3_LEFT.v[1], VEC3_LEFT.v[2], dy * 0.01);
		yaw.from_axis_angle(VEC3_UP.v[0], VEC3_UP.v[1], VEC3_UP.v[2], dx * 0.01);
		pitch = pitch * yaw;
		q_bale_ori = pitch * q_bale_ori;
	};

	std::function<void(void)> bale_prepare = [&]()
	{
		seen::ShaderProgram* shader = seen::Shaders[bale_shader];
		seen::ShaderProgram::active(shader);
		bale_mat->use(&seen::ShaderProgram::active()->draw_params.material_uniforms.tex);

		GLint material_uniform = glGetUniformLocation(shader->program, "material");
		GLint albedo_uniform = glGetUniformLocation(shader->program, "albedo");

		vec4 material = { 0.1, 0.01, 1, 0.01 };
		vec4 albedo = { 1, 1, 1, 1 };

		glUniform4fv(material_uniform, 1, (GLfloat*)material);
		glUniform4fv(albedo_uniform, 1, (GLfloat*)albedo);

		mat4x4 world;
		mat3x3 rot;

		mat4x4_from_quat(world, q_bale_ori.v);

		for(int i = 3; i--;)
		for(int j = 3; j--;)
		{
			rot[i][j] = world[i][j];
		}

	    seen::DrawParams& params = seen::ShaderProgram::active()->draw_params;

		glUniformMatrix4fv(params.world_uniform, 1, GL_FALSE, (GLfloat*)world);
		glUniformMatrix3fv(params.norm_uniform,  1, GL_FALSE, (GLfloat*)rot);
	};
	bale_pass.preparation_function = &bale_prepare;

	std::function<void()> sky_prepare = [&]
	{
		// set the sky shader to active
		seen::ShaderProgram::active(seen::Shaders[sky_shader]);
	};
	sky_pass.preparation_function = &sky_prepare;

	seen::ShaderProgram::active(seen::Shaders[sky_shader]);

	// Add all things to be drawn to the scene
	bale_pass.drawables = new std::vector<seen::Drawable*>();
	bale_pass.drawables->push_back(bale);

	sky_pass.drawables = new std::vector<seen::Drawable*>();
	sky_pass.drawables->push_back(&sky);

	scene.drawables().push_back(&sky_pass);
	scene.drawables().push_back(&bale_pass);

	while(renderer.is_running())
	{
		renderer.draw(&camera, &scene);
	}

	return 0;
}
