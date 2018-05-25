#pragma once

#include "core.h"

namespace seen
{

typedef GLuint Tex;

union Material {
	struct {
		Tex color, normal, specular;
	} textures;
	Tex v[3];
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
	static Tex create_texture(int width, int height, GLenum format, void* data);
	static Tex load_texture(std::string path);
	static int load_texture_buffer(
		std::string path,
		void** data,
		int& width,
		int& height,
		int &depth);
	static Material* get_material(const std::string path);
};

}
