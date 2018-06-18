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


struct Shader {
	enum VarRole {
		VAR_IN = 0,
		VAR_OUT,
		VAR_INOUT,
		VAR_PARAM,
		VAR_LOCAL,
	};

	enum FeatureFlags {
		VERT_POSITION = 1,
		VERT_UV       = 2,
		VERT_NORMAL   = 4,
		VERT_TANGENT  = 8,
	};

	struct Expression {
		std::string str;

		Expression operator+ (Expression e);
		Expression operator+= (Expression e);
		Expression operator- (Expression e);
		Expression operator-= (Expression e);
		Expression operator* (Expression e);
		Expression operator*= (Expression e);
		Expression operator/ (Expression e);
		Expression operator/= (Expression e);
		Expression operator= (Expression e);
		Expression operator== (Expression e);
		Expression operator< (Expression e);
		Expression operator> (Expression e);
		Expression operator<= (Expression e);
		Expression operator>= (Expression e);
		Expression operator<< (Expression e);
		Expression operator>> (Expression e);

		Expression operator[] (std::string swizzel);

		const char* cstr();
	};

	struct Variable : public Expression {
		friend struct Shader;

		Variable() = default;
		Variable(VarRole role, std::string type, std::string name);

		VarRole role;
		std::string name;
		std::string type;

		Variable& as(std::string type);
		Variable& array(int dims);
		std::string declaration();

		Expression at_index(int i);

		Variable& operator<< (Variable property);
		Expression operator[] (std::string lookup);
		Expression operator= (Expression e);
		Expression operator= (Variable v);

	private:
		std::map<std::string, Variable> properties;
		int array_size;
	};

	GLint compiled_shader;
	GLenum type;
	std::string name;

	std::vector<Variable> inputs, outputs, parameters, locals;
	std::vector<Expression> statements;

	Shader(std::string name, GLenum type);
	~Shader();

	Variable& input(std::string name);
	Variable& output(std::string name);
	Variable& parameter(std::string name);
	Variable& local(std::string name);

	Variable* has_variable(std::string name, std::vector<Variable>& vars);
	Variable* has_input(std::string name);

	// high level shader describers
	Shader& preceded_by(Shader& shader);

	Shader& vertex(int feature_flags);
	Shader& viewed();
	Shader& projected();
	Shader& transformed();
	Shader& pass_through(std::string name);

	Shader& color_textured();


	struct {
		Shader& ndl();
	} lighting;

	Expression builtin(std::string gl_name);
	Expression call(std::string func_name, std::vector<Expression> params);
	std::string code();
	GLint compile();

	void next(Expression e);

	static Shader vertex(std::string name);
	static Shader tessalation_control(std::string name);
	static Shader tessalation_evaluation(std::string name);
	static Shader geometry(std::string name);
	static Shader fragment(std::string name);

	static std::string mat(int rank);
	static std::string vec(int rank);
	static std::string tex(int rank);
	static Expression mat(int rank, const char* fmt, ...);
	static Expression vec(int rank, const char* fmt, ...);

	static std::string integer();
	static std::string shortint();

private:
	std::string suffix();
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

	static ShaderProgram* compile(std::vector<Shader> shaders);
private:
	std::map<std::string, ShaderParam*> _params;
	int _tex_counter;
};

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
	friend struct Shader;
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
