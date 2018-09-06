#include "custompass.hpp"
#include "shader.hpp"

#include <stdarg.h>

using namespace seen;

CustomPass::CustomPass()
{
	std::function<void(int)> nop = [&](int){};
	preparation_function = nop;

	instances = 1;
};

CustomPass::CustomPass(std::function<void(int)> prep)
{
	preparation_function = prep;
	instances = 1;
};

CustomPass::~CustomPass() {};


void CustomPass::prepare(int index)
{
	preparation_function(index);
}


void CustomPass::draw(Viewer* viewer)
{
	if (!gl_get_error())
	{
		std::cerr << "Something bad happened before drawing" << std::endl;
	}

	if(!gl_get_error())
	{
		std::cerr << "ERROR: GL error produced in preparation_function" << std::endl;
	}

	for (int i = 0; i < instances; i++)
	{
		prepare(i);

		if(viewer && i == 0)
		{
			ShaderProgram& shader = *ShaderProgram::active();
			shader["u_view_matrix"] << viewer->_view;
			shader["u_proj_matrix"] << viewer->_projection;

		}

		for(auto drawable : scene->all())
		{
			drawable->draw();
		}
	}
}


ShadowPass::ShadowPass(int resolution, bool generate_mipmaps)
{
	_cubemap = new Cubemap(resolution, Framebuffer::depth_flag);
	_generate_mipmaps = generate_mipmaps;

	if (_generate_mipmaps)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_LOD, 3);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LOD, 3);
	}
}


ShadowPass::~ShadowPass()
{
	delete _cubemap;
}


void ShadowPass::prepare(int index)
{
	ShaderProgram::builtin_shadow_depth().use();
	_cubemap->prepare(0);
}


void ShadowPass::draw(Viewer* viewer)
{
	if (!gl_get_error())
	{
		std::cerr << "Something bad happend before drawing" << std::endl;
	}

	prepare(0);

	if(!gl_get_error())
	{
		std::cerr << "ERROR: GL error produced in preparation_function" << std::endl;
	}

	static std::vector<Drawable*> empty;
	for (Light* l : lights)
	{
		_cubemap->side_projection = l->projection;
		_cubemap->draw_at(l->position, scene, empty);
	}
}


void ShadowPass::finish()
{
	_cubemap->finish();

	if (_generate_mipmaps)
	{
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
