#pragma once

#include "core.h"

namespace seen
{

struct DrawParams {
	GLint world_uniform;
	GLint norm_uniform;
	GLint view_uniform;
	GLint proj_uniform;
	struct {
		GLint tex;
		GLint norm;
		GLint spec;
		GLint envd;
	} material_uniforms;
};


struct ShaderConfig {
	std::string vertex;
	std::string fragment;

	const char** vertex_attributes;
};

struct ShaderProgram;
struct ShaderProgram {
	GLint program;
	DrawParams draw_params;

	void init_draw_params();

	static ShaderProgram* active(ShaderProgram* program);
	static ShaderProgram* active();
};


class ShaderCache {
public:
	ShaderCache();
	~ShaderCache();

	ShaderProgram* operator[](ShaderConfig config);

private:
	std::string _shader_path;
	std::map<std::string, GLint> _shader_cache;
	std::map<std::string, ShaderProgram> _program_cache;
};

extern ShaderCache Shaders;

}
