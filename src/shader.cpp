#include "shader.hpp"

seen::ShaderCache seen::Shaders;

using namespace seen;

static GLint load_shader(const char* path, GLenum type)
{
	GLuint shader;
	GLint status = GL_TRUE;
	GLchar *source;

	int fd = open(path, O_RDONLY);

	std::cerr << "compiling '" << path << "'... ";

	if (fd < 0)
	{
		fprintf(stderr, "Failed to load vertex shader '%s' %d\n", path, errno);
		exit(-1);
	}

	// Load the shader source code
	size_t total_size = lseek(fd, 0, SEEK_END);
	source = (GLchar*)calloc(total_size, 1);
	lseek(fd, 0, SEEK_SET);
	read(fd, source, total_size);

	assert(gl_get_error());

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
		std::cerr << "Shader compile log for '" <<  path << "' " << log_length << std::endl << log_str << std::endl;
		write(1, log_str, log_length);
		free(log_str);
	}

	assert(gl_get_error());

	// Check the status and exit on failure
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		std::cerr << "Compiling '" << path << "' failed: " << status << std::endl;
		glDeleteShader(shader);
		exit(-2);
	}

	assert(gl_get_error());

	free(source);

	std::cerr << "ok" << std::endl;

	return shader;
}
//------------------------------------------------------------------------------

static GLint link_program(GLint vertex, GLint frag, const char** attributes)
{
	GLint status;
	GLint prog = glCreateProgram();

	assert(gl_get_error());

	glAttachShader(prog, vertex);
	glAttachShader(prog, frag);

	assert(gl_get_error());

	const char** attr = attributes;
	for(int i = 0; *attr; ++i)
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

	assert(gl_get_error());

	glDetachShader(prog, vertex);
	glDetachShader(prog, frag);
	glDeleteShader(vertex);
	glDeleteShader(frag);

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

	if(shader)
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

	for(int i = 3; i--;)
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

	if(_program_cache.count(name) <= 0)
	{
		std::string vsh_path = DATA_PATH + "/" + config.vertex;
		std::string fsh_path = DATA_PATH + "/" + config.fragment;

		// Load, compile and store the vertex shader if needed
		if(_shader_cache.count(vsh_path) <= 0)
		{
			_shader_cache[vsh_path] = load_shader(vsh_path.c_str(), GL_VERTEX_SHADER);
		}

		// Load, compile and store the fragment shader if needed
		if(_shader_cache.count(fsh_path) <= 0)
		{
			_shader_cache[fsh_path] = load_shader(fsh_path.c_str(), GL_FRAGMENT_SHADER);
		}

		if (!config.vertex_attributes)
		{
			const char* attrs[] = {
				"position", "normal", "tangent", "texcoord", NULL
			};

			config.vertex_attributes = attrs;
		}

		GLint vsh = _shader_cache[vsh_path];
		GLint fsh = _shader_cache[fsh_path];
		GLint program = link_program(vsh, fsh, config.vertex_attributes);

		ShaderProgram shader = {};
		shader.program = program;

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
	assert(gl_get_error());
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
