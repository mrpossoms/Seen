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


Vec3 seen::rn()
{
	Vec3 n(rf() - 0.5f, rf() - 0.5f, rf() - 0.5f);
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

Vec3 Positionable::position()
{
	return _position;
}


Vec3 Positionable::left()
{
	vec4 out;

	quat q;
	quat_invert(q, _orientation.v);
	quat_mul_vec3(out, q, VEC3_LEFT.v);

	return Vec3(-out[0], -out[1], -out[2]);
}


Vec3 Positionable::forward()
{
	vec4 out;

	quat q;
	quat_invert(q, _orientation.v);
	quat_mul_vec3(out, q, VEC3_FORWARD.v);

	return Vec3(-out[0], -out[1], -out[2]);
}


Quat Positionable::orientation()
{
	return _orientation;
}


Positionable* Positionable::position(Vec3& pos)
{
	_position = pos;

	mat4x4 rot, trans;
	mat4x4_from_quat(rot, _orientation.v);

	mat4x4_identity(trans);
	mat4x4_translate_in_place(trans, -_position.x, -_position.y, -_position.z);

	mat4x4_mul(_world.v, rot, trans);

	return this;
}


Positionable* Positionable::position(float x, float y, float z)
{
	Vec3 pos(x, y, z);
	return this->position(pos);
}


Positionable* Positionable::orientation(Quat& ori)
{
	_orientation = ori;

	// mat4x4_from_quat(_view.v, _orientation.v);
	// mat4x4_translate_in_place(_view.v, _position.x, _position.y, _position.z);
	mat4x4 rot, trans;
	mat4x4_from_quat(rot, _orientation.v);

	mat4x4_identity(trans);
	mat4x4_translate_in_place(trans, -_position.x, -_position.y, -_position.z);
	mat4x4_mul(_world.v, rot, trans);

	vec3_copy(normal_matrix.c0, rot[0]);
	vec3_copy(normal_matrix.c1, rot[1]);
	vec3_copy(normal_matrix.c2, rot[2]);

	return this;
}


void Positionable::orientation(mat3x3 rot)
{
	Quat ori;
	mat4x4 m = {};

	vec3_copy(m[0], rot[0]);
	vec3_copy(m[1], rot[1]);
	vec3_copy(m[2], rot[2]);

	quat_from_mat4x4(ori.v, m);

	orientation(ori);
}


void Positionable::world(mat4x4 world)
{
	quat_from_mat4x4(_orientation.v, world);
	_position = Vec3(world[3][0], world[3][1], world[3][2]);
	mat4x4_dup(_world.v, world);
}


mat4x4_t Positionable::world()
{
	return _world;
}
