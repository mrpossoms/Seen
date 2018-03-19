#pragma once

#include "core.h"

namespace seen
{

union Material {
	struct {
		GLuint color, normal, specular;
	} textures;
	GLuint v[3];

	void use(GLint* material_uniforms);
};

struct Framebuffer {
	GLuint color;
	GLuint depth;
	GLuint id;

	static const int color_flag = 1;
	static const int depth_flag = 2;
};

class TextureFactory
{
public:
	static Framebuffer create_framebuffer(int width, int height, int flags);
	static GLuint create_texture(int width, int height, GLenum format, void* data);
	static GLuint load_texture(std::string path);
	static Material* get_material(const std::string path);
};

}
