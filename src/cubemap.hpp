#pragma once

#include "core.h"
#include "texture.hpp"

namespace seen
{

class Cubemap : public RenderingPass {
	friend struct ShaderParam;

public:
	Cubemap(int size);
	Cubemap(int size, int fbo_flags);
	~Cubemap();
	void init(int size, int fbo_flags);

	void prepare(int index);

	void draw(Viewer* viewer) {};
	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding);
	void draw_at(vec<3> position, Scene* scene, std::vector<Drawable*>& excluding);

	void finish();

	mat<4,4> side_projection;
private:
	GLuint _map;
	GLint _last_viewport[4];

	Framebuffer _framebuffer;
	int _size;

	void render_to(GLenum face);
};

}
