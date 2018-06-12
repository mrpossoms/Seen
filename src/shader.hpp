#pragma once

#include "core.h"
#include "texture.hpp"

namespace seen
{
struct ShaderProgram;

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
	struct {
		std::string control;
		std::string evaluation;
	} tessalation;
	std::string geometry;
	std::string fragment;

	const char** vertex_attributes;
};


struct ShaderParam {
	ShaderParam(ShaderProgram* program, const char* name);

	void operator<<(float f);
	void operator<<(vec3_t& v);
	void operator<<(vec4_t& v);
	void operator<<(mat3x3_t& m);
	void operator<<(mat4x4_t& m);
	void operator<<(GLint i);
	void operator<<(Tex t);
private:
	ShaderProgram* _program;
	GLint _uniform;
};


struct ShaderProgram {
	friend struct ShaderParam;

	GLint program;
	GLint primative;
	DrawParams draw_params;

	void init_draw_params();
	ShaderProgram* use();

	ShaderParam& operator[](std::string name);

	void operator<<(Material* m);

	static ShaderProgram* active(ShaderProgram* program);
	static ShaderProgram* active();
private:
	std::map<std::string, ShaderParam*> _params;
	int _tex_counter;
};


struct PipeVariable {
	std::string layout;
	std::string type;
	std::string name;

	bool operator ==(PipeVariable& var);
};


struct Pipe {
	std::vector<PipeVariable> in;
	std::vector<PipeVariable> out;

	struct {
		std::vector<PipeVariable> provides;
		std::vector<PipeVariable> needs;
	} locals;

	struct {
		std::vector<PipeVariable> provides;
		std::vector<PipeVariable> needs;
	} parameters;

	std::string code;

	bool is_satisfied();

	GLint compile();

protected:
	Pipe *prev, *next;
	GLenum shader_type;

	void append(Pipe* p);
};


struct FragmentPipe : public Pipe {
	FragmentPipe* position();
	FragmentPipe* normal();
	FragmentPipe* tangent();
	FragmentPipe* binormal();
	FragmentPipe* texture();
};


struct RasterPipe : public Pipe {
	RasterPipe(FragmentPipe* fragment);
	RasterPipe* lit();
	RasterPipe* specular();
	RasterPipe* colored(Vec3 color);
	RasterPipe* textured();
};


struct TessallationPipe : public Pipe {
	TessallationPipe* transformed();
	TessallationPipe* projected();
};


struct VertexPipe : public Pipe {
	VertexPipe* position();
	VertexPipe* normal();
	VertexPipe* tangent();
	VertexPipe* texture();

protected:

};


struct GeomertyPipe : public Pipe {
	GeomertyPipe(VertexPipe* vertex);

	GeomertyPipe* transformed();
	GeomertyPipe* projected();

	TessallationPipe* tessalated();

	RasterPipe* yeilds_fragment(FragmentPipe* frag);
};

// auto vertex = Shader::vertex().position().texture().normal().tangent()
// auto pipeline = vertex.transformed().projected()
// or..
// auto pipeline = vertex.tessalated().transformed().projected()

// auto pipeline = Shader::vertex(Shader::position().texture().normal().tangent())
//                 .tessalated().transformed().projected()

// auto pipeline = Shader::vertex(Position | Texture | Normal | Tangent)
//                 .tessalated().transformed().projected()


// auto vsh = Shader::Vertex("basic_vsh");
// auto position = vsh.input("position_in").as(Shader::vec(3));
// auto uv_in = vsh.input("texcoord_in").as(Shader::vec(2));
// auto uv_out = vsh.output("texcoord_vsh").as(Shader::vec(2));
// auto mvp = vsh.parameter("uModelViewProjection").as(Shader::mat(4));
// vsh.next() << uv_out = uv_in;
// vsh.next() << vsh.builtin("gl_Position") = mvp * position;
// vsh.compile()
//
// auto fsh = Shader::Fragment("basic_fsh");
// auto tex = fsh.parameter("uTexture").as(Shader::tex2d);
// auto uv_in = fsh.input("texcoord_vsh").as(Shader::vec(2));
// fsh.next() << fsh.builtin("gl_Color")["rgba"] = fsh.call("texture", tex, uv_in);
// fsh.compile()

// auto program = Shader::compile(vsh, fsh);

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
