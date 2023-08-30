#pragma once
#include "core.h"

namespace seen
{

class Camera : public Viewer {
public:
	Camera(float fov, int frame_w, int frame_h);

	// Positionable
	vec<3> position();
	Positionable* position(const vec<3>& pos);
	Positionable* position(float x, float y, float z);

	quat<> orientation();
	Positionable* orientation(const quat<>& ori);

	void rotation(mat<3, 3> rot);
	void matrix(mat<4, 4> world);

	Viewer* view_projection(mat<4, 4>& vp);
	Viewer* view(const mat<4, 4>& v);
	Viewer* projection(const mat<4, 4>& p);

	Viewer* fov(float f);

	vec<3> left();
	vec<3> forward();
	vec<3> up();
	// mat<4, 4> view();
	// mat<4, 4> projection();

private:
	vec<3> _position;
	quat<> _orientation;
	// mat<4, 4> _projection, _view;
	int width, height;
};

}
