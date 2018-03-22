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

extern std::string DATA_PATH;

bool gl_get_error();

}
