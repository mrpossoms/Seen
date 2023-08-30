#include "camera.hpp"

using namespace seen;

Camera::Camera(float fov,
			   int frame_w,
			   int frame_h)
{
	width = frame_w;
	height = frame_h;

	float aspect = width / (float)height;

	_projection = mat<4, 4>::perspective(0.01, 1000, fov, aspect);

	position(0, 0, 0);
}


Viewer* Camera::view_projection(mat<4, 4>& vp)
{
	mat<4, 4> view = _orientation.to_matrix();
	vec<3> pos = position();
	view *= mat<4, 4>::translation({-pos[0], -pos[1], -pos[2]});

	vp = _projection *  view;

	return this;
}


vec<3> Camera::position()
{
	return _position;
}

Viewer* Camera::fov(float f)
{
	float aspect = width / (float)height;

	_projection = mat<4, 4>::perspective(0.01, 1000, f, aspect);

	return this;
}


vec<3> Camera::left()
{
	// TODO: define LEFT in xmath.h
	return _orientation.inverse().rotate({-1, 0, 0});
}


vec<3> Camera::forward()
{
	// TODO: define FORWARD in xmath.h
	return _orientation.inverse().rotate({0, 0, -1});
}


vec<3> Camera::up()
{
	// TODO: define UP in xmath.h
	return _orientation.inverse().rotate({0, 1, 0});
}


Positionable* Camera::position(const vec<3>& pos)
{
	_position = pos;

	_view = _orientation.to_matrix();
	_view *= mat<4, 4>::translation({-_position[0], -_position[1], -_position[2]});
	// mat<4, 4>_from_quat(_view.v, _orientation.v);
	// mat<4,4>ranslate_in_place(_view.v, -_position[0], -_position[1], -_position[2]);

	return this;
}


Positionable* Camera::position(float x, float y, float z)
{
		_position[0] = x;
		_position[1] = y;
		_position[2] = z;

		// mat<4, 4> rot, trans;
		// mat<4, 4>_from_quat(rot, _orientation.v);

		// mat<4, 4>_identity(trans);
		// mat<4,4>ranslate_in_place(trans, -_position[0], -_position[1], -_position[2]);

		// mat<4, 4>_mul(_view.v, rot, trans);

		auto rot = _orientation.to_matrix();
		auto trans = mat<4, 4>::translation({-_position[0], -_position[1], -_position[2]});

		_view = rot * trans;

		return this;
}


quat<> Camera::orientation()
{
	return _orientation;
}


Positionable* Camera::orientation(const quat<>& ori)
{
	_orientation = ori;

	// mat<4, 4>_from_quat(_view.v, _orientation.v);
	// mat<4,4>ranslate_in_place(_view.v, _position[0], _position[1], _position[2]);
	mat<4, 4> rot = ori.to_matrix(), trans = mat<4, 4>::translation(-_position);
	//mat<4, 4>_from_quat(rot, _orientation.v);

	// mat<4, 4>_identity(trans);
	// mat<4,4>ranslate_in_place(trans, -_position[0], -_position[1], -_position[2]);

	// mat<4, 4>_mul(_view.v, rot, trans);
	_view = rot * trans;

	return this;
}


void Camera::rotation(mat<3, 3> rot) {}
void Camera::matrix(mat<4, 4> world) {}


Viewer* Camera::view(const mat<4, 4>& v)
{
	_view = v;

	return this;
}


Viewer* Camera::projection(const mat<4, 4>& p)
{
	_projection = p;

	return this;
}
