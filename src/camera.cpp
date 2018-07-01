#include "camera.hpp"

using namespace seen;

Camera::Camera(float fov,
			   int frame_w,
			   int frame_h)
{
	width = frame_w;
	height = frame_h;

	float aspect = width / (float)height;

	mat4x4_perspective(_projection.v, fov, aspect, 0.01, 1000);
	position(0, 0, 0);
}


Viewer* Camera::view_projection(mat4x4 vp)
{
	mat4x4 view;

	Vec3 pos = position();
	mat4x4_from_quat(view, orientation().v);
	mat4x4_translate_in_place(view, -pos.x, -pos.y, -pos.z);

	mat4x4_mul(vp, _projection.v, view);

	return this;
}


Vec3 Camera::position()
{
	return _position;
}

Viewer* Camera::fov(float f)
{
	float aspect = width / (float)height;

	mat4x4_perspective(_projection.v, f, aspect, 0.01, 1000);

	return this;
}


Vec3 Camera::left()
{
	vec4 out;

	quat q;
	quat_invert(q, _orientation.v);
	quat_mul_vec3(out, q, VEC3_LEFT.v);

	return Vec3(-out[0], -out[1], -out[2]);
}


Vec3 Camera::forward()
{
	vec4 out;

	quat q;
	quat_invert(q, _orientation.v);
	quat_mul_vec3(out, q, VEC3_FORWARD.v);

	return Vec3(-out[0], -out[1], -out[2]);
}


Vec3 Camera::up()
{
	vec4 out;

	quat q;
	quat_invert(q, _orientation.v);
	quat_mul_vec3(out, q, VEC3_DOWN.v);

	return Vec3(-out[0], -out[1], -out[2]);
}


Positionable* Camera::position(Vec3& pos)
{
	_position = pos;

	mat4x4_from_quat(_view.v, _orientation.v);
	mat4x4_translate_in_place(_view.v, -_position.x, -_position.y, -_position.z);

	return this;
}


Positionable* Camera::position(float x, float y, float z)
{
		_position.x = x;
		_position.y = y;
		_position.z = z;

		mat4x4 rot, trans;
		mat4x4_from_quat(rot, _orientation.v);

		mat4x4_identity(trans);
		mat4x4_translate_in_place(trans, -_position.x, -_position.y, -_position.z);

		mat4x4_mul(_view.v, rot, trans);
		return this;
}


Quat Camera::orientation()
{
	return _orientation;
}


Positionable* Camera::orientation(Quat& ori)
{
	_orientation = ori;

	// mat4x4_from_quat(_view.v, _orientation.v);
	// mat4x4_translate_in_place(_view.v, _position.x, _position.y, _position.z);
	mat4x4 rot, trans;
	mat4x4_from_quat(rot, _orientation.v);

	mat4x4_identity(trans);
	mat4x4_translate_in_place(trans, -_position.x, -_position.y, -_position.z);

	mat4x4_mul(_view.v, rot, trans);

	return this;
}


void Camera::rotation(mat3x3 rot) {}
void Camera::matrix(mat4x4 world) {}


Viewer* Camera::view(mat4x4 v)
{
	mat4x4_dup(_view.v, v);

	return this;
}


Viewer* Camera::projection(mat4x4 p)
{
	mat4x4_dup(p, _projection.v);

	return this;
}
