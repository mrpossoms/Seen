#include "renderergl.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "geo.hpp"
#include "shader.hpp"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

using namespace seen;

std::string seen::DATA_PATH;

//------------------------------------------------------------------------------
static GLFWwindow* init_glfw(int width, int height, const char* title)
{
	if (!glfwInit()){
		fprintf(stderr, "glfwInit() failed\n");
		exit(-1);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* win = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!win){
		glfwTerminate();
		fprintf(stderr, "glfwCreateWindow() failed\n");
		exit(-2);
	}

	glfwMakeContextCurrent(win);
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	assert(gl_get_error());

	return win;
}
//------------------------------------------------------------------------------

RendererGL::RendererGL(const char* data_path, const char* title)
{
	width = 640;
	height = 480;

	DATA_PATH = std::string(data_path);

	win = init_glfw(width, height, title);

	assert(gl_get_error());
}
//------------------------------------------------------------------------------

bool RendererGL::is_running()
{
	return !(glfwWindowShouldClose(win) || glfwGetKey(win, GLFW_KEY_ESCAPE));
}
//------------------------------------------------------------------------------

void RendererGL::prepare()
{
	// Set the screen's framebuffer as the render target
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	assert(gl_get_error());

	// reset the viewport
	// Make the sky black, and clear the screen
	int fb_width, fb_height;
	glfwGetFramebufferSize(win, &fb_width, &fb_height);
	glViewport(0, 0, fb_width, fb_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	assert(gl_get_error());
}
//------------------------------------------------------------------------------

void RendererGL::draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding)
{
	assert(gl_get_error());

	prepare();

	if(viewer)
	{
		DrawParams& params = ShaderProgram::active()->draw_params;

		glUniformMatrix4fv(params.view_uniform, 1, GL_FALSE, (GLfloat*)viewer->_view);
		glUniformMatrix4fv(params.proj_uniform,  1, GL_FALSE, (GLfloat*)viewer->_projection);

		assert(gl_get_error());
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (scene)
	for(auto drawable : scene->drawables())
	{
		// skip it drawable is in excluding
		if (std::find(excluding.begin(), excluding.end(), drawable) != excluding.end())
			continue;

		drawable->draw(viewer);
	}

	assert(gl_get_error());

	glfwPollEvents();
	glfwSwapBuffers(win);

	double xpos, ypos;
	glfwGetCursorPos(win, &xpos, &ypos);
	mouse_moved(xpos, ypos, xpos - mouse_last_x, ypos - mouse_last_y);
	mouse_last_x = xpos;
	mouse_last_y = ypos;
}
//------------------------------------------------------------------------------

void RendererGL::draw(Viewer* viewer, Scene* scene)
{
	std::vector<Drawable*> empty;

	draw(viewer, scene, empty);
}
//------------------------------------------------------------------------------

void RendererGL::finish()
{

}
