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

CustomPass::CustomPass(std::function<void(int)> prep, ...)
{
	va_list args;

	va_start(args, prep);
	while(true)
	{
		Drawable* d = va_arg(args, Drawable*);
		if (!d) break;
		drawables.push_back(d);
	}
	va_end(args);

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
		std::cerr << "Something bad happend before drawing" << std::endl;
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
			//DrawParams& params = ShaderProgram::active()->draw_params;

			//glUniformMatrix4fv(params.view_uniform, 1, GL_FALSE, (GLfloat*)viewer->_view);
			//glUniformMatrix4fv(params.proj_uniform,  1, GL_FALSE, (GLfloat*)viewer->_projection);

			ShaderProgram& shader = *ShaderProgram::active();
			shader["u_view_matrix"] << viewer->_view;
			shader["u_proj_matrix"] << viewer->_projection;

		}

		for(auto drawable : scene->drawables())
		{
			drawable->draw();
		}
	}
}


ShadowPass::ShadowPass(int resolution)
{
	_cubemap = new Cubemap(resolution, Framebuffer::depth_flag);
}


ShadowPass::~ShadowPass()
{
	delete _cubemap;
}


void ShadowPass::prepare(int index)
{
	ShaderProgram::builtin_red()->use();
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
		ShaderProgram::builtin_red()->use();

		_cubemap->draw_at(l->position, scene, empty);
	}
}


void ShadowPass::finish()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
