#pragma once

#include "core.h"
#include "texture.hpp"
#include "light.hpp"
#include "custompass.hpp"

GLint link_program(const GLint* shaders, const char** attributes);
GLint load_shader(const char* path, GLenum type);
GLint compile_source(const char* src, GLenum type);

namespace seen
{

struct ShaderProgram;

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
	void operator<<(Cubemap* c);
private:
	ShaderProgram* _program;
	GLint _uniform;
};


struct Shader {
	friend struct ShaderProgram;

	enum class VarRole {
		VAR_IN,
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

	struct Code {
		std::string str;
		const char* cstr();
	};

	struct Expression : public Code {

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
		Expression length();
		Expression cross(Expression e);
		Expression pow(float power);
		Expression mix(std::vector<Expression> params, float percent);
		Expression mix(std::vector<Expression> params, Expression percent);
		Expression saturate();

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
		Expression operator= (float f);

		static void copy(Variable& src, Variable& dst);
	private:
		std::map<std::string, Variable> properties;
		int array_size;
	};

	GLint compiled_shader;
	GLenum type;
	std::string name;

	std::vector<Variable> inputs, outputs, parameters, locals;
	std::vector<Code> statements;

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

	Shader& color_white();
	Shader& color_textured();
	Shader& blinn();
	Shader& normal_mapped();
	Shader& shadow_mapped(bool for_point_light=true);
	Shader& shadow_mapped_vsm(bool for_point_light=true);


	Expression vec2(float x, float y);
	Expression vec3(float x, float y, float z);
	Expression vec4(float x, float y, float z, float w);

	Expression mat3(Expression c0, Expression c1, Expression c2);

	struct {
		Shader& ndl();
	} lighting;

	Expression builtin(std::string gl_name);
	Expression call(std::string func_name, std::vector<Expression> params);
	std::string code();
	GLint compile();

	Shader& next(Expression e);
	Shader& next_if(Expression e, std::function<void (void)> then);

	static Shader vertex(std::string name);
	static Shader tessalation_control(std::string name);
	static Shader tessalation_evaluation(std::string name);
	static Shader geometry(std::string name);
	static Shader fragment(std::string name);

	static std::string mat(int rank);
	static std::string vec(int rank);
	static std::string tex(int rank);
	static std::string cubemap();
	static std::string shadowCube();
	static Expression mat(int rank, const char* fmt, ...);
	static Expression vec(int rank, const char* fmt, ...);

	static std::string integer();
	static std::string shortint();

private:
	std::string suffix();
	int _code_block;
};


struct ShaderProgram {
	friend struct ShaderParam;

	GLint program;
	GLint primative;
	std::string name;

	ShaderProgram& use();

	ShaderParam& operator[](std::string name);

	void operator<<(Material* m);
	void operator<<(Viewer* v);
	void operator<<(Positionable* p);
	void operator<<(Light* l);
	void operator<<(ShadowPass* s);

	static ShaderProgram* active(ShaderProgram* program);
	static ShaderProgram* active();

	static ShaderProgram& compile(std::string name, std::vector<Shader> shaders);
	static ShaderProgram& get(std::string name);

	static ShaderProgram& builtin_sky();
	static ShaderProgram& builtin_realistic();
	static ShaderProgram& builtin_shadow_depth();
	static ShaderProgram& builtin_normal_colors();
private:
	std::map<std::string, ShaderParam*> _params;
	int _tex_counter;
};


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
