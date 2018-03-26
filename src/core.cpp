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
