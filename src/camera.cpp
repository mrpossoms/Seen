#include "camera.hpp"

using namespace seen;

Camera::Camera(float fov,
			   int frame_w,
			   int frame_h)
{
	width = frame_w;
	height = frame_h;

	float aspect = width / (float)height;

	printf("%d x %d : %f\n", width, height, aspect);

	mat4x4_perspective(_projection, fov, aspect, 0.01, 1000);
}


Viewer* Camera::view_projection(mat4x4 vp)
{
	mat4x4 view;

	Vec3 pos = position();
	mat4x4_from_quat(view, orientation().v);
	mat4x4_translate_in_place(view, -pos.x, -pos.y, -pos.z);

	mat4x4_mul(vp, _projection, view);

	return this;
}


Vec3 Camera::position()
{
	return _position;
}


Positionable* Camera::position(Vec3& pos)
{
	_position = pos;

	mat4x4_from_quat(_view, _orientation.v);
	mat4x4_translate(_view, _position.x, _position.y, _position.z);

	return this;
}


Positionable* Camera::position(float x, float y, float z)
{
		_position.x = x;
		_position.y = y;
		_position.z = z;

		mat4x4_from_quat(_view, _orientation.v);
		mat4x4_translate(_view, _position.x, _position.y, _position.z);

		return this;
}


Quat Camera::orientation()
{
	return _orientation;
}


Positionable* Camera::orientation(Quat& ori)
{
	_orientation = ori;

	mat4x4_from_quat(_view, _orientation.v);
	mat4x4_translate(_view, _position.x, _position.y, _position.z);

	return this;
}


void Camera::rotation(mat3x3 rot) {}
void Camera::matrix(mat4x4 world) {}


Viewer* Camera::view(mat4x4 v)
{
	mat4x4_dup(_view, v);

	return this;
}


Viewer* Camera::projection(mat4x4 p)
{
	mat4x4_dup(p, _projection);

	return this;
}
