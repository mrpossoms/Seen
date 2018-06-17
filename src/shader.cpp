#include "shader.hpp"
#include "renderergl.hpp"
#include <iomanip>

seen::ShaderCache seen::Shaders;

using namespace seen;


static GLint compile_source(const char* src, GLenum type)
{
	GLuint shader;
	GLint status = GL_TRUE;
	GLchar *source = (GLchar*)src;

	std::cerr << "compiling... ";

	// Create the GL shader and attempt to compile it
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	assert(gl_get_error());

	// Print the compilation log if there's anything in there
	GLint log_length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
	if (log_length > 0)
	{
		GLchar *log_str = (GLchar *)malloc(log_length);
		glGetShaderInfoLog(shader, log_length, &log_length, log_str);
		std::cerr << SEEN_TERM_RED "Shader compile log: " << log_length << std::endl << log_str << SEEN_TERM_COLOR_OFF << std::endl;
		write(1, log_str, log_length);
		free(log_str);
	}

	assert(gl_get_error());

	// Check the status and exit on failure
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cerr << SEEN_TERM_RED "Compiling failed: " << status << SEEN_TERM_COLOR_OFF << std::endl;
		glDeleteShader(shader);
		exit(-2);
	}

	assert(gl_get_error());
	std::cerr << SEEN_TERM_GREEN "OK" SEEN_TERM_COLOR_OFF << std::endl;

	return shader;
}


static GLint load_shader(const char* path, GLenum type)
{
	GLuint shader;
	GLchar *source;

	std::cerr << path << ": ";

	int fd = open(path, O_RDONLY);

	if (fd < 0)
	{
		fprintf(stderr, SEEN_TERM_RED "Failed to load vertex shader '%s' %d\n" SEEN_TERM_COLOR_OFF, path, errno);
		exit(-1);
	}

	// Load the shader source code
	size_t total_size = lseek(fd, 0, SEEK_END);
	source = (GLchar*)calloc(1, total_size);
	lseek(fd, 0, SEEK_SET);
	read(fd, source, total_size);

	// compile it
	assert(gl_get_error());
	shader = compile_source(source, type);

	free(source);

	return shader;
}
//------------------------------------------------------------------------------

static GLint link_program(const GLint* shaders, const char** attributes)
{
	GLint status;
	GLint prog = glCreateProgram();

	assert(gl_get_error());

	// Attach all shaders
	for (int i = 0; shaders[i]; ++i)
	{
		glAttachShader(prog, shaders[i]);
	}

	assert(gl_get_error());

	const char** attr = attributes;
	for (int i = 0; *attr; ++i)
	{
		glBindAttribLocation(prog, i, *attr);
		++attr;
	}

	assert(gl_get_error());

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status == 0)
	{
		GLint log_length;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_length);
		if (log_length > 0)
		{
			GLchar *log_str = (GLchar *)malloc(log_length);
			glGetProgramInfoLog(prog, log_length, &log_length, log_str);
			std::cerr << "Shader link log: " << log_length << std::endl << log_str << std::endl;
			write(1, log_str, log_length);
			free(log_str);
		}
		exit(-1);
	}
	else
	{
		std::cerr << SEEN_TERM_GREEN "Linked program " << prog << SEEN_TERM_COLOR_OFF << std::endl;
	}

	assert(gl_get_error());

	// Detach all
	for (int i = 0; shaders[i]; ++i)
	{
		glDetachShader(prog, shaders[i]);
	}

	return prog;
}
//------------------------------------------------------------------------------

void ShaderProgram::init_draw_params()
{
	//draw_params.world_uniform = glGetUniformLocation(program, "u_world_matrix");
	//draw_params.norm_uniform  = glGetUniformLocation(program, "u_normal_matrix");
	//draw_params.view_uniform  = glGetUniformLocation(program, "view_matrix");
	//draw_params.proj_uniform  = glGetUniformLocation(program, "proj_matrix");

	std::string uniforms[] = {
		"u_world_matrix",
		"u_normal_matrix",
		"u_view_matrix",
		"u_proj_matrix"
	};

	for (auto uniform : uniforms)
	{
		(*this)[uniform];
	}

	draw_params.material_uniforms.tex  = glGetUniformLocation(program, "tex");
	draw_params.material_uniforms.norm = glGetUniformLocation(program, "norm");
	draw_params.material_uniforms.spec = glGetUniformLocation(program, "spec");
	draw_params.material_uniforms.envd = glGetUniformLocation(program, "envd");

	gl_get_error();
}
//------------------------------------------------------------------------------

ShaderProgram* ShaderProgram::active(ShaderProgram* shader)
{
	static ShaderProgram* active;

	assert(gl_get_error());

	if (shader)
	{
		glUseProgram(shader->program);
		active = shader;
	}

	assert(gl_get_error());
	assert(active);

	return active;
}
//------------------------------------------------------------------------------

ShaderProgram* ShaderProgram::active()
{
	return ShaderProgram::active(NULL);
}
//------------------------------------------------------------------------------

ShaderProgram* ShaderProgram::compile(std::vector<Shader> shaders)
{
	const char* attributes[16] = {};
	GLint gs_shaders[6] = {};
	ShaderProgram* program = new ShaderProgram();

	// compile, cache and include each shader
	int si = 0;
	for (auto shader : shaders)
	{
		if (shader.type == GL_VERTEX_SHADER)
		{
			for (int i = 0; i < shader.inputs.size(); i++)
			{
				attributes[i] = shader.inputs[i].name.c_str();
			}
		}

		gs_shaders[si += 1] = shader.compile();
	}

	program->program = link_program(gs_shaders, attributes);

	// preload the uniforms
	for (auto shader : shaders)
	{
		for (auto param : shader.parameters)
		{
			(*program)[param.name];
		}
	}

	std::cerr << "prog: " << program->program << std::endl;

	return program;
}

ShaderProgram* ShaderProgram::use()
{
	_tex_counter = 0; // reset texture location
	ShaderProgram::active(this);
	return this;
}
//------------------------------------------------------------------------------

void ShaderProgram::operator<<(Material* m)
{
	const std::string uniform_names[] = {
		"us_color",
		"us_normal",
		"us_specular"
	};

	for (int i = 3; i--;)
	{
		glActiveTexture(GL_TEXTURE0 + _tex_counter);
		glBindTexture(GL_TEXTURE_2D, m->v[i]);
		(*this)[uniform_names[i]] << _tex_counter;
		_tex_counter++;
	}
}
//------------------------------------------------------------------------------

ShaderCache::ShaderCache()
{

}
//------------------------------------------------------------------------------

ShaderCache::~ShaderCache()
{

}
//------------------------------------------------------------------------------

ShaderProgram* ShaderCache::operator[](ShaderConfig config)
{
	std::string name = config.vertex + config.fragment;

	if (_program_cache.count(name) <= 0)
	{
		std::string shader_names[] = {
			config.vertex,
			config.tessalation.control,
			config.tessalation.evaluation,
			config.geometry,
			config.fragment
		};

		GLenum shader_types[] = {
			GL_VERTEX_SHADER,
			GL_TESS_CONTROL_SHADER,
			GL_TESS_EVALUATION_SHADER,
			GL_GEOMETRY_SHADER,
			GL_FRAGMENT_SHADER
		};

		// Iterate over all specified shader names
		for (unsigned int i = 0; i < sizeof(shader_types) / sizeof(GLenum); ++i)
		{
			std::string& name = shader_names[i];
			if (name.length() == 0) continue;

			std::string path = DATA_PATH + "/" + name;

			// Load, compile and store the vertex shader if needed
			if (_shader_cache.count(path) <= 0)
			{
				_shader_cache[name] = load_shader(path.c_str(), shader_types[i]);
			}
		}

		// If no attributes were provided, use the defaults
		if (!config.vertex_attributes)
		{
			const char* attrs[] = {
				"a_position", "a_normal", "a_tangent", "a_texcoord", NULL
			};

			config.vertex_attributes = attrs;
		}

		int si = 0;
		GLint shaders[6] = {};
		for (unsigned int i = 0; i < sizeof(shader_types) / sizeof(GLenum); ++i)
		{
			if (shader_names[i].length())
			{
				shaders[si++] = _shader_cache[shader_names[i]];
			}
		}

		GLint program = link_program(shaders, config.vertex_attributes);

		ShaderProgram shader = {};
		shader.program = program;
		shader.primative = GL_TRIANGLES;

		if (config.tessalation.evaluation.length() > 0)
		{
			shader.primative = GL_PATCHES;
		}

		ShaderProgram::active(&shader);
		shader.init_draw_params();

		_program_cache[name] = shader;
	}

	ShaderProgram::active(&_program_cache[name]);

	return &_program_cache[name];
}
//------------------------------------------------------------------------------

ShaderParam& ShaderProgram::operator[](std::string name)
{
	if (_params.count(name) == 0)
	{
		_params[name] = new ShaderParam(this, name.c_str());
	}

	return *_params[name];
}
//------------------------------------------------------------------------------

ShaderParam::ShaderParam(ShaderProgram* program, const char* name)
{
	bool is_good = gl_get_error();
	if(!is_good)
	{
		std::cerr << SEEN_TERM_RED "Error befor setting: " << name << SEEN_TERM_COLOR_OFF << '\n';
	}
	assert(is_good);

	_program = program;
	_uniform = glGetUniformLocation(_program->program, name);
	assert(gl_get_error());
}
//------------------------------------------------------------------------------

void ShaderParam::operator<<(float f)
{
	glUniform1f(_uniform, f);
}
//------------------------------------------------------------------------------

void ShaderParam::operator<<(vec3_t& v)
{
	glUniform3fv(_uniform, 1, (GLfloat*)v.v);
}
//------------------------------------------------------------------------------

void ShaderParam::operator<<(vec4_t& v)
{
	glUniform4fv(_uniform, 1, (GLfloat*)v.v);
}
//------------------------------------------------------------------------------

void ShaderParam::operator<<(mat3x3_t& m)
{
	glUniformMatrix3fv(_uniform, 1, GL_FALSE, (GLfloat*)m.v);
}
//------------------------------------------------------------------------------

void ShaderParam::operator<<(mat4x4_t& m)
{
	glUniformMatrix4fv(_uniform, 1, GL_FALSE, (GLfloat*)m.v);
}
//------------------------------------------------------------------------------

void ShaderParam::operator<<(GLint i)
{
	glUniform1i(_uniform, i);
}
//------------------------------------------------------------------------------

void ShaderParam::operator<<(Tex t)
{
	glActiveTexture(GL_TEXTURE0 + _program->_tex_counter);
	glBindTexture(GL_TEXTURE_2D, t);
	glUniform1i(_uniform, _program->_tex_counter);
	_program->_tex_counter++;
}


std::string Shader::mat(int rank) { return "mat" + std::to_string(rank); }
std::string Shader::vec(int rank)
{
	if (rank == 1)
	{
		return "float";
	}

	return "vec" + std::to_string(rank);
}
std::string Shader::integer() { return "int"; }
std::string Shader::shortint() { return "short"; }

Shader::Expression Shader::mat(int rank, const char* fmt, ...)
{
	char args[128];

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(args, sizeof(args), fmt, ap);
	va_end(ap);

	return { Shader::mat(rank) + "(" + std::string(args) + ")" };
}

Shader::Expression Shader::vec(int rank, const char* fmt, ...)
{
	char args[128];

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(args, sizeof(args), fmt, ap);
	va_end(ap);

	return { Shader::vec(rank) + "(" + std::string(args) + ")" };
}

Shader::Expression Shader::Expression::operator+ (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " + " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator+= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " += " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator- (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " - " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator-= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " -= " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator* (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " * " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator*= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " *= " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator/ (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " / " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator/= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " /= " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " = " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator== (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " == " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator< (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " < " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator> (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " > " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator<= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " <= " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator>= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " >= " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator<< (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " << " + e.str };
	return eo;
}

Shader::Expression Shader::Expression::operator>> (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " >> " + e.str };
	return eo;
}


Shader::Expression Shader::Expression::operator[] (std::string swizzel)
{
	Shader::Expression eo = { this->str + "." + swizzel };
	return eo;
}


const char* Shader::Expression::cstr() { return str.c_str(); }


Shader::Variable::Variable(VarRole role, std::string type, std::string name)
{
	this->role = role;
	this->type = type;
	this->name = name;
	this->str = name;
}


Shader::Variable& Shader::Variable::as(std::string type)
{
	this->type = type;
	return *this;
}


Shader::Variable& Shader::Variable::array(int dims)
{
	array_size = dims;
	return *this;
}


std::string Shader::Variable::declaration()
{
	std::string rank = "";

	if (array_size > 0)
	{
		rank = "[" + std::to_string(array_size) + "]";
	}

	switch (role)
	{
		case VAR_IN:
			return "in " + this->type + " " + this->name + rank;
			break;
		case VAR_OUT:
			return "out " + this->type + " " + this->name + rank;
			break;
		case VAR_PARAM:
			return "uniform " + this->type + " " + this->name + rank;
			break;
	}
}


Shader::Expression Shader::Variable::at_index(int i)
{
	assert(array_size > 0);

	return { str + "[" + std::to_string(i) + "]" };
}


void Shader::next(Shader::Expression e)
{
	statements.push_back(e);
}


Shader::Variable& Shader::input(std::string name)
{
	Shader::Variable var(VAR_IN, "", name);
	inputs.push_back(var);
	return inputs[inputs.size() - 1];
}


Shader::Variable& Shader::output(std::string name)
{
	Shader::Variable var(VAR_OUT, "", name);
	outputs.push_back(var);
	return outputs[outputs.size() - 1];
}


Shader::Variable& Shader::parameter(std::string name)
{
	Shader::Variable var(VAR_PARAM, "", name);
	parameters.push_back(var);
	return parameters[parameters.size() - 1];
}


Shader::Variable& Shader::Variable::operator<< (Shader::Variable property)
{
	if (type == "struct")
	{
		properties[property.name] = property;
	}

	return *this;
}


Shader::Expression Shader::Variable::operator[] (std::string lookup)
{
	if (type == "struct")
	{
		return properties[lookup];
	}

	return { str + "." + lookup };
}


Shader::Expression Shader::Variable::operator= (Shader::Expression e)
{
	return { str + " = " + e.str };
}


Shader::Expression Shader::Variable::operator= (Shader::Variable e)
{
	return { str + " = " + e.str };
}


Shader::Shader(std::string name, GLenum type)
{
	this->name = name;
	this->type = type;
}


Shader::~Shader()
{

}


Shader::Expression Shader::builtin(std::string gl_name)
{
	static std::map<std::string, Shader::Variable> vertex_builtin_map = {
		{ "gl_VertexID",     Shader::Variable(VAR_IN, "int", "gl_VertexID") },
		{ "gl_InstanceID",   Shader::Variable(VAR_IN, "int", "gl_InstanceID") },
		{ "gl_DrawID",       Shader::Variable(VAR_IN, "int", "gl_DrawID") },
		{ "gl_BaseVertex",   Shader::Variable(VAR_IN, "int", "gl_BaseVertex") },
		{ "gl_BaseInstance", Shader::Variable(VAR_IN, "int", "gl_BaseInstance") },

		{ "gl_Position",     Shader::Variable(VAR_OUT, "vec4", "gl_Position") },
		{ "gl_PointSize",    Shader::Variable(VAR_OUT, "float", "gl_PointSize") },
	};

	static std::map<std::string, Shader::Variable> tess_control_builtin_map = {
		{ "gl_PatchVerticesIn", Shader::Variable(VAR_IN, "int", "gl_PatchVerticesIn") },
		{ "gl_PrimitiveID",     Shader::Variable(VAR_IN, "int", "gl_PrimitiveID") },
		{ "gl_InvocationID",    Shader::Variable(VAR_IN, "int", "gl_InvocationID") },
		{ "gl_in", Shader::Variable(VAR_IN, "struct", "gl_in").array(32)
		                                                       << Shader::Variable(VAR_IN, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VAR_IN, "float", "gl_PointSize")
		                                                       << Shader::Variable(VAR_IN, "float", "gl_ClipDistance").array(32)},

		{ "gl_TessLevelOuter",     Shader::Variable(VAR_OUT, "float", "gl_TessLevelOuter").array(4) },
		{ "gl_TessLevelInner",    Shader::Variable(VAR_OUT, "float", "gl_TessLevelInner").array(2) },
		{ "gl_out", Shader::Variable(VAR_OUT, "struct", "gl_out").array(32)
		                                                       << Shader::Variable(VAR_OUT, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VAR_OUT, "float", "gl_PointSize")
		                                                       << Shader::Variable(VAR_OUT, "float", "gl_ClipDistance").array(32)},
	};

	static std::map<std::string, Shader::Variable> tess_eval_builtin_map = {
		{ "gl_TessCoord", Shader::Variable(VAR_IN, "vec3", "gl_TessCoord") },
		{ "gl_PatchVerticesIn",     Shader::Variable(VAR_IN, "int", "gl_PatchVerticesIn") },
		{ "gl_PrimitiveID",    Shader::Variable(VAR_IN, "int", "gl_PrimitiveID") },
		{ "gl_TessLevelOuter",     Shader::Variable(VAR_IN, "float", "gl_TessLevelOuter").array(4) },
		{ "gl_TessLevelInner",    Shader::Variable(VAR_IN, "float", "gl_TessLevelInner").array(2) },
		{ "gl_in", Shader::Variable(VAR_IN, "struct", "gl_in").array(32)
		                                                       << Shader::Variable(VAR_IN, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VAR_IN, "float", "gl_PointSize")
		                                                       << Shader::Variable(VAR_IN, "float", "gl_ClipDistance").array(32)},

		{ "gl_Position",     Shader::Variable(VAR_OUT, "vec4", "gl_Position") },
		{ "gl_PointSize",    Shader::Variable(VAR_OUT, "float", "gl_PointSize") },
		{ "gl_ClipDistance", Shader::Variable(VAR_OUT, "float", "gl_ClipDistance").array(32) },
	};

	static std::map<std::string, Shader::Variable> geometry_builtin_map = {
		{ "gl_in", Shader::Variable(VAR_IN, "struct", "gl_in").array(32)
		                                                       << Shader::Variable(VAR_IN, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VAR_IN, "float", "gl_PointSize")
		                                                       << Shader::Variable(VAR_IN, "float", "gl_ClipDistance").array(32)},
		{ "gl_PrimitiveIDIn",     Shader::Variable(VAR_IN, "int", "gl_PrimitiveIDIn") },
		{ "gl_InvocationID",    Shader::Variable(VAR_IN, "int", "gl_InvocationID") },

		{ "gl_Position",     Shader::Variable(VAR_OUT, "vec4", "gl_Position") },
		{ "gl_PointSize",    Shader::Variable(VAR_OUT, "float", "gl_PointSize") },
		{ "gl_ClipDistance", Shader::Variable(VAR_OUT, "float", "gl_ClipDistance").array(32) },
	};

	static std::map<std::string, Shader::Variable> fragment_builtin_map = {
		{ "gl_FragCoord",     Shader::Variable(VAR_IN, "vec4", "gl_FragCoord") },
		{ "gl_FrontFacing",   Shader::Variable(VAR_IN, "bool", "gl_FrontFacing") },
		{ "gl_PointCoord",    Shader::Variable(VAR_IN, "vec2", "gl_PointCoord") },
		{ "gl_SampleID",      Shader::Variable(VAR_IN, "int", "gl_SampleID") },
		{ "gl_PointCoord",    Shader::Variable(VAR_IN, "vec2", "gl_PointCoord") },
		{ "gl_SampleMaskIn",  Shader::Variable(VAR_IN, "int", "gl_SampleMaskIn") },
		{ "gl_ClipDistance",  Shader::Variable(VAR_IN, "float", "gl_ClipDistance").array(32) },
		{ "gl_PrimitiveID",   Shader::Variable(VAR_IN, "int", "gl_PrimitiveID") },
		{ "gl_Layer",         Shader::Variable(VAR_IN, "int", "gl_Layer") },
		{ "gl_ViewportIndex", Shader::Variable(VAR_IN, "int", "gl_ViewportIndex") },

		{ "gl_FragDepth",     Shader::Variable(VAR_OUT, "float", "gl_FragDepth") },
		{ "gl_Color",         Shader::Variable(VAR_OUT, "vec4", "gl_Color") },
		{ "gl_SampleMask",    Shader::Variable(VAR_OUT, "int", "gl_SampleMask").array(32) },
	};

	switch (type)
	{
		case GL_VERTEX_SHADER:
			return vertex_builtin_map[gl_name];
		case GL_TESS_CONTROL_SHADER:
			return tess_control_builtin_map[gl_name];
		case GL_TESS_EVALUATION_SHADER:
			return tess_eval_builtin_map[gl_name];
		case GL_GEOMETRY_SHADER:
			return geometry_builtin_map[gl_name];
		case GL_FRAGMENT_SHADER:
			return fragment_builtin_map[gl_name];
	}

	static Shader::Variable empty{};
	return empty;
}


Shader::Expression Shader::call(std::string func_name, std::vector<Expression> params)
{
	Expression call = { func_name + "(" };

	for (int i = 0; i < params.size(); i++)
	{
		call.str += params[i].str;

		if (i < params.size() - 1)
		{
			call.str += ", ";
		}
	}

	call.str += ")";

	return call;
}


std::string Shader::code()
{
	std::stringstream src;

	if (RendererGL::version_major || RendererGL::version_minor)
	{
		src << "#version " << RendererGL::version_major << std::setfill('0') << std::setw(2) << (RendererGL::version_minor * 10) << std::endl;
		src << std::endl;
	}

	auto emit_var_list = [&](std::vector<Variable> vars) {
		for (int i = 0; i < vars.size(); i++)
		{
			src << vars[i].declaration() << ";" << std::endl;
		}
	};

	switch (type)
	{
		case GL_VERTEX_SHADER:
			for (int i = 0; i < inputs.size(); i++)
			{
				src << "layout(location = " << std::to_string(i) << ") " << inputs[i].declaration() << ";" << std::endl;
			}
			src << std::endl;
			emit_var_list(outputs);
			break;
		case GL_TESS_CONTROL_SHADER:
			src << "layout(vertices = 3) out;" << std::endl;
			src << std::endl;
			emit_var_list(inputs);
			src << std::endl;
			emit_var_list(outputs);
			break;
		case GL_TESS_EVALUATION_SHADER:
			src << "layout(triangles, equal_spacing, ccw) in;" << std::endl;
			src << std::endl;
			emit_var_list(inputs);
			src << std::endl;
			emit_var_list(outputs);
			break;
		case GL_GEOMETRY_SHADER:
			src << "layout(triangles) in;" << std::endl;
			src << "layout(triangle_strip, max_vertices = MAX_VERTS) out;" << std::endl;
			src << std::endl;
			emit_var_list(inputs);
			src << std::endl;
			emit_var_list(outputs);
			break;
		case GL_FRAGMENT_SHADER:
			emit_var_list(inputs);
			src << std::endl;
			emit_var_list(outputs);
			break;
	}

	src << std::endl;

	for (auto param : parameters)
	{
		src << param.declaration() << ";" << std::endl;
	}

	src << std::endl;
	src << "void main()" << std::endl;
	src << "{" << std::endl;
	for (auto statement : statements)
	{
		src << "\t" << statement.str << ";" << std::endl;
	}
	src << "}" << std::endl;

	return src.str();
}


GLint Shader::compile()
{
	GLint shader = compile_source(code().c_str(), type);
	Shaders._shader_cache[name] = shader;

	return shader;
}


Shader Shader::vertex(std::string name)
{
	Shader shader(name, GL_VERTEX_SHADER);
	return shader;
}


Shader Shader::tessalation_control(std::string name)
{
	Shader shader(name, GL_TESS_CONTROL_SHADER);
	return shader;
}


Shader Shader::tessalation_evaluation(std::string name)
{
	Shader shader(name, GL_TESS_EVALUATION_SHADER);
	return shader;
}


Shader Shader::geometry(std::string name)
{
	Shader shader(name, GL_GEOMETRY_SHADER);
	return shader;
}


Shader Shader::fragment(std::string name)
{
	Shader shader(name, GL_FRAGMENT_SHADER);
	return shader;
}
