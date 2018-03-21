#pragma once

// system libs
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

// c libs
#include "string.h"

// c++ libs
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <streambuf>
#include <algorithm>

// project libs
#ifdef __linux__
#include <GL/glx.h>
#include <GL/glext.h>
#endif

#include <GLFW/glfw3.h>

#ifdef __APPLE__
#undef __gl_h_
#include <OpenGL/gl3.h>
#endif


// #include <GL/glew.h>
#include <linmath.h>
#include "interfaces.hpp"

namespace seen {


static bool gl_get_error()
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

static char* str_from_file(const char* path)
{
	char* str = NULL;

	int fd = open(path, O_RDONLY);
	assert(fd);

	size_t file_size = lseek(fd, SEEK_END, 0);
	str = (char*)malloc(file_size);
	assert(str);

	assert(read(fd, str, file_size) == file_size);
	close(fd);

	return str;
}

}
