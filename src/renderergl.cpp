#include "renderergl.hpp"
#include "texture.hpp"
#include "geo.hpp"
#include "shader.hpp"

#include <png.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#endif

using namespace seen;

std::string seen::DATA_PATH;

static RendererGL* SINGLTON;

int RendererGL::version_major = 0;
int RendererGL::version_minor = 0;

static void key_callback(GLFWwindow* window,
                         int key,
                         int scancode,
                         int action,
                         int mods)
{
	if (action == GLFW_PRESS)
	{
		SINGLTON->keys_down[key] = 1;
	}
	else if (action == GLFW_RELEASE)
	{
        if (SINGLTON->keys_down[key])
        {
            SINGLTON->key_released(key);
        }

		SINGLTON->keys_down[key] = 0;
	}
}

//------------------------------------------------------------------------------
static GLFWwindow* init_glfw(int width, int height, const char* title, int version[2])
{
	if (!glfwInit()){
		fprintf(stderr, "glfwInit() failed\n");
		exit(-1);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);

	if (version[0] | version[1])
	{
		RendererGL::version_major = version[0];
		RendererGL::version_minor = version[1];

		std::cerr << "Requesting GL " << version[0] << "." << version[1] << std::endl;
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version[0]);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version[1]);

		if (version[0] >= 3)
		{
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		}
		else
		{
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
		}

	}

	//glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);

	GLFWwindow* win = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!win)
	{
		glfwTerminate();
		fprintf(stderr, "glfwCreateWindow() failed\n");
		exit(-2);
	}

	glfwMakeContextCurrent(win);
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(win, key_callback);

	if (version[0] >= 3)
	{
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		assert(gl_get_error());
	}

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	assert(gl_get_error());

	return win;
}
//------------------------------------------------------------------------------

RendererGL::RendererGL(
	const char* data_path,
	const char* title,
	int win_w,
	int win_h,
	int gl_version_major,
	int gl_version_minor)
{
	width = win_w;
	height = win_h;

	DATA_PATH = std::string(data_path);

	int version[] = { gl_version_major, gl_version_minor };
	win = init_glfw(width, height, title, version);

	// NOP by default
	mouse_moved  = [&](double x, double y, double dx, double dy) { };
	key_pressed  = [&](int key) { };
    key_released = [&](int key) { };

	assert(gl_get_error());

	SINGLTON = this;
}
//------------------------------------------------------------------------------

bool RendererGL::is_running()
{
	return !(glfwWindowShouldClose(win) || glfwGetKey(win, GLFW_KEY_ESCAPE));
}
//------------------------------------------------------------------------------

void RendererGL::prepare(int index)
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

void RendererGL::draw(Viewer* viewer, std::vector<RenderingPass*> passes)
{
	assert(gl_get_error());

	prepare(-1);

	if(viewer)
	{
		//DrawParams& params = ShaderProgram::active()->draw_params;

		//glUniformMatrix4fv(params.view_uniform, 1, GL_FALSE, (GLfloat*)viewer->_view);
		//glUniformMatrix4fv(params.proj_uniform,  1, GL_FALSE, (GLfloat*)viewer->_projection);
		//(*ShaderProgram::active())["u_view_matrix"] << ((mat4x4_t)viewer->_view);


		assert(gl_get_error());
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int key = 1024; key--;)
	{
		if (keys_down[key] == 1)
		{
			key_pressed(key);
		}
	}

	for(auto pass : passes)
	{
		//pass->prepare(0);
		pass->draw(viewer);
		pass->finish();
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

void RendererGL::finish()
{

}
//------------------------------------------------------------------------------

static void write_png_file_rgb(
	const char* path,
	int width,
	int height,
	const GLchar* buffer){

	FILE *fp = fopen(path, "wb");

	if(!fp) abort();

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();

	png_infop info = png_create_info_struct(png);
	if (!info) abort();

	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	// Output is 8bit depth, RGB format.
	png_set_IHDR(
		png,
		info,
		width, height,
		8,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	png_bytep rows[height];
	for(int i = height; i--;)
	{
		rows[(height - 1) - i] = (png_bytep)(buffer + i * (width * 3));
	}

	png_write_image(png, rows);
	png_write_end(png, NULL);

	fclose(fp);
}
//------------------------------------------------------------------------------

bool RendererGL::capture(std::string path)
{
	int fb_width, fb_height;
	glfwGetFramebufferSize(win, &fb_width, &fb_height);

	size_t buf_len = fb_width * fb_height * 3;
	GLchar color_buffer[buf_len];
	bzero(color_buffer, buf_len);

	glReadPixels(
		0, 0,
		fb_width, fb_height,
		GL_RGB, GL_UNSIGNED_BYTE,
		(void*)color_buffer
	);

	write_png_file_rgb(path.c_str(), fb_width, fb_height, color_buffer);

	return true;
}
//------------------------------------------------------------------------------

void RendererGL::use_free_cam(Camera& cam)
{
	mouse_moved = [&](double x, double y, double dx, double dy)
	{
		static bool skip;

		if (!skip)
		{
			skip = true;
			return;
		}

		Quat pitch, yaw, roll;
		Vec3 forward, left, up;

		auto q = cam.orientation();

		pitch.from_axis_angle(VEC3_LEFT.v[0], VEC3_LEFT.v[1], VEC3_LEFT.v[2], dy * 0.01);
		yaw.from_axis_angle(VEC3_UP.v[0], VEC3_UP.v[1], VEC3_UP.v[2], dx * 0.01);
		pitch = pitch * yaw;
		q = pitch * q;

		cam.orientation(q);
	};

	key_pressed = [&](int key)
	{
		Vec3 new_pos = cam.position();
		Quat roll, q = cam.orientation();

		switch (key)
		{
			case GLFW_KEY_S:
				new_pos -= cam.forward() * 0.1;
				break;
			case GLFW_KEY_W:
				new_pos += cam.forward() * 0.1;
				break;
			case GLFW_KEY_A:
				new_pos += cam.left() * 0.1;
				break;
			case GLFW_KEY_D:
				new_pos -= cam.left() * 0.1;
				break;
			case GLFW_KEY_Q:
				roll.from_axis_angle(0, 0, 1, -0.05);
				break;
			case GLFW_KEY_E:
				roll.from_axis_angle(0, 0, 1, 0.05);
				break;
			case GLFW_KEY_SPACE:
				new_pos += cam.up() * 0.1;
				break;
			case GLFW_KEY_LEFT_SHIFT:
			case GLFW_KEY_LEFT_CONTROL:
				new_pos -= cam.up() * 0.1;
				break;
		}

		roll = roll * q;
		cam.orientation(roll);
		cam.position(new_pos);
	};
}
//------------------------------------------------------------------------------

void RendererGL::clear_color(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}
