#include "custompass.hpp"
#include "shader.hpp"

using namespace seen;

CustomPass::CustomPass() {};

CustomPass::~CustomPass() {};


void CustomPass::prepare()
{
	if (preparation_function)
	{
		(*preparation_function)();
	}
}


void CustomPass::draw(Viewer* viewer)
{
	assert(viewer);

	prepare();

	if(viewer)
	{
		DrawParams& params = ShaderProgram::active()->draw_params;

		glUniformMatrix4fv(params.view_uniform, 1, GL_FALSE, (GLfloat*)viewer->_view);
		glUniformMatrix4fv(params.proj_uniform,  1, GL_FALSE, (GLfloat*)viewer->_projection);

		assert(gl_get_error());
	}

	for(auto drawable : *drawables)
	{
		drawable->draw(viewer);
	}
}
