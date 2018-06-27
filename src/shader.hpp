#pragma once

#include "core.h"
#include "texture.hpp"

GLint link_program(const GLint* shaders, const char** attributes);
GLint load_shader(const char* path, GLenum type);
GLint compile_source(const char* src, GLenum type);

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
	std::string fragment;
	struct {
		std::string control;
		std::string evaluation;
	} tessalation;
	std::string geometry;

	const char** vertex_attributes;
};


struct ShaderParam {
	ShaderParam(ShaderProgram* program, const char* name);

	void operator<<(float f);
	void operator<<(vec3_t& v);
	void operator<<(vec4_t& v);
	void operator<<(Vec3 v);
	// void operator<<(Vec4 v);
	void operator<<(mat3x3_t& m);
	void operator<<(mat4x4_t& m);
	void operator<<(GLint i);
	void operator<<(Tex t);
private:
	ShaderProgram* _program;
	GLint _uniform;
};


struct Shader {
	friend struct ShaderProgram;

	enum VarRole {
		VAR_IN = 0,
		VAR_OUT,
		VAR_INOUT,
		VAR_PARAM,
		VAR_LOCAL,
		VAR_NONE,
	};

	enum FeatureFlags {
		VERT_POSITION = 1,
		VERT_UV       = 2,
		VERT_NORMAL   = 4,
		VERT_TANGENT  = 8,
	};

	struct Expression {
		std::string str;

		Expression(std::string s);
		Expression() = default;

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

		Expression operator+ (std::string e);
		Expression operator+= (std::string e);
		Expression operator- (std::string e);
		Expression operator-= (std::string e);
		Expression operator* (std::string e);
		Expression operator*= (std::string e);
		Expression operator/ (std::string e);
		Expression operator/= (std::string e);
		Expression operator= (std::string e);

		Expression operator+ (float e);
		Expression operator+= (float e);
		Expression operator- (float e);
		Expression operator-= (float e);
		Expression operator* (float e);
		Expression operator*= (float e);
		Expression operator/ (float e);
		Expression operator/= (float e);
		Expression operator= (float e);
		Expression operator< (float e);
		Expression operator> (float e);
		Expression operator<= (float e);
		Expression operator>= (float e);

		Expression operator[] (std::string swizzel);

		Expression normalize();
		Expression dot(Expression e);
		Expression cross(Expression e);
		Expression pow(float power);
		Expression mix(std::vector<Expression> params, float percent);
		Expression mix(std::vector<Expression> params, Expression percent);
		Expression saturate();

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
	Variable* has_output(std::string name);

	// high level shader describers
	Shader& preceded_by(Shader& shader);

	Shader& vertex(int feature_flags);
	Shader& viewed();
	Shader& projected();
	Shader& transformed();
	Shader& compute_binormal();
	Shader& emit_position();
	Shader& pass_through(std::string name);

	Shader& color_textured();
	Shader& blinn();

	Expression vec2(float x, float y);
	Expression vec3(float x, float y, float z);
	Expression vec4(float x, float y, float z, float w);


	struct {
		Shader& ndl();
	} lighting;

	Expression builtin(std::string gl_name);
	Expression call(std::string func_name, std::vector<Expression> params);
	std::string code();
	GLint compile();

	Shader& next(Expression e);

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
	void operator<<(Viewer* v);
	void operator<<(Positionable* p);

	static ShaderProgram* active(ShaderProgram* program);
	static ShaderProgram* active();

	static ShaderProgram* compile(std::vector<Shader> shaders);

	static ShaderProgram* builtin_sky();
	static ShaderProgram* builtin_normal_colors();
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
	friend struct ShaderProgram;
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
