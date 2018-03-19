#include "envmap.hpp"
#include "shader.hpp"

using namespace seen;

//------------------------------------------------------------------------------

EnvironmentMap::EnvironmentMap(int size)
{
	_size = size;

	GLenum sides[] = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	};

	glGenTextures(1, &_map);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _map);

	assert(gl_get_error());

	for(int i = 6; i--;)
	{
		glTexImage2D(
			sides[i],
			0,
			GL_RGB,
			_size, _size,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			NULL);
	}

	assert(gl_get_error());

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	assert(gl_get_error());

	_framebuffer = TextureFactory::create_framebuffer(
		size, size,
		Framebuffer::depth_flag | Framebuffer::color_flag
	);
}
//------------------------------------------------------------------------------

void EnvironmentMap::render_to(GLenum face)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, _map);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer.id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, _map, 0);
	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}
//------------------------------------------------------------------------------

void EnvironmentMap::prepare()
{
	glViewport(0, 0, _size, _size);
}
//------------------------------------------------------------------------------

void EnvironmentMap::draw(Viewer* viewer,
                          Scene* scene,
                          std::vector<Drawable*> excluding)
{
	Vec3 pos = viewer->position();
	draw_at(pos, scene, excluding);
}
//------------------------------------------------------------------------------

void EnvironmentMap::draw_at(Vec3 position,
                             Scene* scene,
                             std::vector<Drawable*> excluding)
{
	struct basis {
		Vec3 up, forward;
	};

	const GLenum sides[] = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	};

	const basis bases[] = {
		{ VEC3_UP,      VEC3_BACK    },
		{ VEC3_UP,      VEC3_FORWARD },
		{ VEC3_FORWARD, VEC3_UP      },
		{ VEC3_BACK,    VEC3_DOWN    },
		{ VEC3_DOWN,    VEC3_RIGHT   },
		{ VEC3_DOWN,    VEC3_LEFT    },
	};
	mat4x4 cube_views[6];

	static bool setup;
	static mat4x4 cube_proj;

	// Compute perspective and view matrices the first time a
	// draw_at call is made
	if(!setup)
	{
		mat4x4_perspective(cube_proj, M_PI / 2, 1, 0.01, 1000);
		setup = true;
	}

	// Compute new view matrices each time, as position could have changed
	for(int i = 6; i--;)
	{
		Vec3 eye = position + bases[i].forward;

		mat4x4_look_at(
			cube_views[i],
			position.v,
			eye.v,
			bases[i].up.v
		);
	}

	assert(gl_get_error());

	DrawParams draw_params = ShaderProgram::active()->draw_params;

	int i = 0;
	for(; i < 6; i++)
	{
		render_to(sides[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(draw_params.view_uniform, 1, GL_FALSE, (GLfloat*)cube_views[i]);
		glUniformMatrix4fv(draw_params.proj_uniform, 1, GL_FALSE, (GLfloat*)cube_proj);

		assert(gl_get_error());

		for(auto drawable : scene->drawables())
		{
			// skip it drawable is in excluding
			if (std::find(excluding.begin(), excluding.end(), drawable) != excluding.end())
				continue;

			drawable->draw(NULL);
		}

		assert(gl_get_error());
	}

	assert(gl_get_error());
}

//------------------------------------------------------------------------------
void EnvironmentMap::finish()
{

}
