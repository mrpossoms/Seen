#pragma once

#include "core.h"
#include <functional>

namespace seen
{

class RendererGL : public Renderer
{
public:
	RendererGL(
		const char* data_path,
		const char* title,
		int win_w=640,
		int win_h=480,
		int gl_version_major=0,
		int gl_version_minor=0);

	void prepare();

	bool is_running();

	void draw(Viewer* viewer) {}
	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding);
	void draw(Viewer* viewer, Scene* scene);

	void finish();
	void clear_color(float r, float g, float b, float a);
	bool capture(std::string path);

	GLFWwindow* win;
	int width, height;

	std::function<void(double x, double y, double dx, double dy)> mouse_moved;
	std::function<void(int key)> key_pressed;
	std::function<void(int key)> key_released;

	char keys_down[1024] = {};
private:
	double mouse_last_x, mouse_last_y;
};

}
