#pragma once
#include "core.h"

namespace seen
{

class Camera : public Viewer {
public:
	Camera(float fov, int frame_w, int frame_h);

	// Positionable
	Vec3 position();
	Positionable* position(Vec3& pos);
	Positionable* position(float x, float y, float z);

	Quat orientation();
	Positionable* orientation(Quat& ori);

	void rotation(mat3x3 rot);
	void matrix(mat4x4 world);

	Viewer* view_projection(mat4x4 vp) override;
	Viewer* view(mat4x4 v) override;
	Viewer* projection(mat4x4 p) override;

	Viewer* fov(float f);

	Vec3 left();
	Vec3 forward();
	Vec3 up();
	// mat4x4 view();
	// mat4x4 projection();

private:
	Vec3 _position;
	Quat _orientation;
	// mat4x4 _projection, _view;
	int width, height;
};

}
