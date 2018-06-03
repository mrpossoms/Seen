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
