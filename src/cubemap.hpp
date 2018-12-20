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

	void prepare(int index) override;
	void draw(Viewer* viewer) override {};
	void finish() override;

	void draw(Viewer* viewer, Scene* scene, std::vector<const Drawable*>& excluding);
	void draw_at(Vec3 position, Scene* scene, std::vector<const Drawable*>& excluding);

	mat4x4_t side_projection;
private:
	GLuint _map;
	GLint _last_viewport[4];

	Framebuffer _framebuffer;
	int _size;

	void render_to(GLenum face);
};

}
