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
