#pragma once
#include "core.h"

namespace seen
{

class Camera : public Viewer {
public:
	Camera(float fov, int frame_w, int frame_h);
	Viewer* view_projection(mat4x4 vp);
	Viewer* view(mat4x4 v);
	Viewer* projection(mat4x4 p);

private:
	mat4x4 _projection;
	int width, height;
};

}
