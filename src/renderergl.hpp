#pragma once

#include "core.h"
#include <functional>

namespace seen
{

class RendererGL : public Renderer
{
public:
	RendererGL(std::string data_path, std::string title);

	void prepare();

	bool is_running();

	void draw(Viewer* viewer) {}
	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding);
	void draw(Viewer* viewer, Scene* scene);

	void finish();

	GLFWwindow* win;
	int width, height;

	std::function<void(double x, double y, double dx, double dy)> mouse_moved;
private:
	double mouse_last_x, mouse_last_y;

};

}
