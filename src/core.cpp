#include "core.h"

bool seen::gl_get_error()
{
	GLenum err = GL_NO_ERROR;
	bool good = true;

	while((err = glGetError()) != GL_NO_ERROR)
	{
		std::cerr << "GL_ERROR: 0x" << std::hex << err << std::endl;
		good = false;
	}

	return good;
}


float seen::rf()
{
	return random() % SEEN_RAND_F_DENOMINATOR / (float)SEEN_RAND_F_DENOMINATOR;
}


float seen::rf(float min, float max)
{
	return rf() * (max - min) + min;
}


vec<3> seen::rn()
{
	vec<3> n(rf() - 0.5f, rf() - 0.5f, rf() - 0.5f);
	n.normalize();
	return n;
}


std::string get_line(std::string lines, int line_number)
{
	size_t pos = 0, end_pos = 0;

	for (int i = 0; i < line_number; i++)
	{
		pos = lines.find("\n", pos);

		if (pos == std::string::npos) goto fail;
	}

	end_pos = lines.find("\n", pos);

	if (end_pos == std::string::npos) goto fail;

	return lines.substr(pos, end_pos);

fail:
	return "";
}


using namespace seen;

vec<3>& Positionable::position()
{
	return _position;
}


vec<3> Positionable::left()
{
	vec4 out;

	quat q;
	quat_invert(q, _orientation.v);
	quat_mul_vec<3>(out, q, VEC3_LEFT.v);

	return vec<3>(-out[0], -out[1], -out[2]);
}


vec<3> Positionable::forward()
{
	vec4 out;

	quat q;
	quat_invert(q, _orientation.v);
	quat_mul_vec<3>(out, q, VEC3_FORWARD.v);

	return vec<3>(-out[0], -out[1], -out[2]);
}


quat Positionable::orientation()
{
	return _orientation;
}


Positionable* Positionable::position(vec<3>& pos)
{
	_position = pos;

	mat<4, 4> rot, trans;
	mat<4, 4>_from_quat(rot, _orientation.v);

	mat<4, 4>_identity(trans);
	mat<4,4>ranslate_in_place(trans, -_position.x, -_position.y, -_position.z);

	mat<4, 4>_mul(_world.v, rot, trans);

	return this;
}


Positionable* Positionable::position(float x, float y, float z)
{
	vec<3> pos(x, y, z);
	return this->position(pos);
}


Positionable* Positionable::orientation(quat& ori)
{
	_orientation = ori;

	// mat<4, 4>_from_quat(_view.v, _orientation.v);
	// mat<4,4>ranslate_in_place(_view.v, _position.x, _position.y, _position.z);
	mat<4, 4> rot, trans;
	mat<4, 4>_from_quat(rot, _orientation.v);

	mat<4, 4>_identity(trans);
	mat<4,4>ranslate_in_place(trans, -_position.x, -_position.y, -_position.z);
	mat<4, 4>_mul(_world.v, rot, trans);

	vec<3>_norm(normal_matrix.c0, rot[0]);
	vec<3>_norm(normal_matrix.c1, rot[1]);
	vec<3>_norm(normal_matrix.c2, rot[2]);

	return this;
}


void Positionable::orientation(mat<3, 3> rot)
{
	quat ori;
	mat<4, 4> m = {};

	vec<3>_copy(m[0], rot[0]);
	vec<3>_copy(m[1], rot[1]);
	vec<3>_copy(m[2], rot[2]);

	quat_from_mat<4, 4>(ori.v, m);

	orientation(ori);
}


void Positionable::world(mat<4, 4> world)
{
	quat_from_mat<4, 4>(_orientation.v, world);
	_position = vec<3>(world[3][0], world[3][1], world[3][2]);
	mat<4, 4>_dup(_world.v, world);
}


mat<4,4>& Positionable::world()
{
	return _world;
}


// Iterator Scene::begin()
// {
// 	Iterator itr(this);
// 	this->iteration_start();

// 	return itr;
// }
