#pragma once

#include "core.h"
#include "camera.hpp"

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

	void prepare(int index);

	bool is_running();

	void finish();
	void clear_color(float r, float g, float b, float a);
	bool capture(std::string path);

	void use_free_cam(Camera& cam);

	void draw(Viewer* viewer, std::vector<RenderingPass*> passes);

	GLFWwindow* win;
	int width, height;

	std::function<void(double x, double y, double dx, double dy)> mouse_moved;
	std::function<void(int key)> key_pressed;
	std::function<void(int key)> key_released;

	char keys_down[1024] = {};

	static int version_major, version_minor;

private:
	double mouse_last_x, mouse_last_y;
};

}
