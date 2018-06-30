#pragma once

#include "core.h"
#include "texture.hpp"

namespace seen
{

class Cubemap : public RenderingPass {
public:
	Cubemap(int size);
	Cubemap(int size, int fbo_flags);
	~Cubemap();
	void init(int size, int fbo_flags);

	void prepare(int index);

	void draw(Viewer* viewer) {};
	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding);
	void draw_at(Vec3 position, Scene* scene, std::vector<Drawable*>& excluding);

	void finish();

private:
	GLuint _map;

	Framebuffer _framebuffer;
	int _size;

	void render_to(GLenum face);
};

}
