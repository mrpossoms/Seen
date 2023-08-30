#include "cubemap.hpp"
#include "shader.hpp"

using namespace seen;

//------------------------------------------------------------------------------

Cubemap::Cubemap(int size)
{
	init(size, Framebuffer::depth_flag | Framebuffer::color_flag);
}
//------------------------------------------------------------------------------

Cubemap::Cubemap(int size, int fbo_flags)
{
	init(size, fbo_flags);
}
//------------------------------------------------------------------------------

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &_map);
}
//------------------------------------------------------------------------------

void Cubemap::init(int size, int fbo_flags)
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

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
			GL_FLOAT, //GL_UNSIGNED_BYTE,
			NULL);
	}

	assert(gl_get_error());

	_framebuffer = TextureFactory::create_framebuffer(
		size, size,
		fbo_flags
	);

	mat<4, 4>_perspective(side_projection.v, M_PI / 2, 1, 0.01, 1000);
}
//------------------------------------------------------------------------------

void Cubemap::render_to(GLenum face)
{
	assert(gl_get_error());

	glBindTexture(GL_TEXTURE_CUBE_MAP, _map);

	assert(gl_get_error());
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer.id);

	assert(gl_get_error());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, face, _map, 0);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	assert(gl_get_error());
}
//------------------------------------------------------------------------------

void Cubemap::prepare(int index)
{
	glGetIntegerv(GL_VIEWPORT, _last_viewport);
	glViewport(0, 0, _size, _size);
}
//------------------------------------------------------------------------------

void Cubemap::draw(Viewer* viewer,
                   Scene* scene,
                   std::vector<Drawable*>& excluding)
{
	vec<3> pos = viewer->position();
	draw_at(pos, scene, excluding);
}
//------------------------------------------------------------------------------

void Cubemap::draw_at(vec<3> position,
                      Scene* scene,
                      std::vector<Drawable*>& excluding)
{
	struct basis {
		vec<3> up, forward;
	};

	const GLenum sides[] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	};

	const basis bases[] = {
		{ VEC3_FORWARD, VEC3_UP      },
		{ VEC3_BACK,    VEC3_DOWN    },
		{ VEC3_DOWN,    VEC3_BACK    },
		{ VEC3_DOWN,    VEC3_FORWARD },
		{ VEC3_DOWN,    VEC3_RIGHT   },
		{ VEC3_DOWN,    VEC3_LEFT    },
	};
	mat<4,4> cube_views[6];

	// Compute new view matrices each time, as position could have changed
	for(int i = 6; i--;)
	{
		vec<3> eye = position + bases[i].forward;

		mat<4, 4>_look_at(
			cube_views[i].v,
			position.v,
			eye.v,
			bases[i].up.v
		);
	}

	assert(gl_get_error());

	auto shader = *ShaderProgram::active();
	shader["u_proj_matrix"] << side_projection;

	int i = 0;
	for(; i < 6; i++)
	{
		render_to(sides[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader["u_view_matrix"] << cube_views[i];

		assert(gl_get_error());

		for(auto drawable : scene->all())
		{
			// skip it drawable is in excluding
			if (std::find(excluding.begin(), excluding.end(), drawable) != excluding.end())
				continue;

			drawable->draw();
		}

		assert(gl_get_error());
	}
	assert(gl_get_error());
}

//------------------------------------------------------------------------------
void Cubemap::finish()
{
	glViewport(_last_viewport[0], _last_viewport[1], _last_viewport[2], _last_viewport[3]);
}
