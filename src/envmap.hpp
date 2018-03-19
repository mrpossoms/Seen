#pragma once

#include "core.h"
#include "texture.hpp"

namespace seen
{

class EnvironmentMap : public RenderingPass {
public:
	EnvironmentMap(int size);

	void prepare();

	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*> excluding);
	void draw_at(Vec3 position, Scene* scene, std::vector<Drawable*> excluding);

	void finish();

private:
	GLuint _map;

	Framebuffer _framebuffer;
	int _size;

	void render_to(GLenum face);

};

}
