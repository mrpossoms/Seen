#include "shader.hpp"

seen::ShaderCache seen::Shaders("./data/");

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
	GLint logLength;
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
	draw_params.world_uniform = glGetUniformLocation(program, "world_matrix");
	draw_params.norm_uniform  = glGetUniformLocation(program, "normal_matrix");
	draw_params.view_uniform  = glGetUniformLocation(program, "view_matrix");
	draw_params.proj_uniform  = glGetUniformLocation(program, "proj_matrix");

	draw_params.material_uniforms.tex  = glGetUniformLocation(program, "tex");
	draw_params.material_uniforms.norm = glGetUniformLocation(program, "norm");
	draw_params.material_uniforms.spec = glGetUniformLocation(program, "spec");
	draw_params.material_uniforms.envd = glGetUniformLocation(program, "envd");
}
//------------------------------------------------------------------------------

ShaderProgram* ShaderProgram::active(ShaderProgram* shader)
{
	static ShaderProgram* active;

	if(shader)
	{
		glUseProgram(shader->program);
		active = shader;
	}

	assert(active);

	return active;
}
//------------------------------------------------------------------------------

ShaderProgram* ShaderProgram::active()
{
	return ShaderProgram::active(NULL);
}
//------------------------------------------------------------------------------

ShaderCache::ShaderCache(std::string shader_path)
{
	_shader_path = shader_path;
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
		std::string vsh_path = _shader_path + "/" + config.vertex;
		std::string fsh_path = _shader_path + "/" + config.fragment;

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

		ShaderProgram shader = {
			.program = program,
		};
		shader.init_draw_params();

		_program_cache[name] = shader;
	}

	return &_program_cache[name];
}
//------------------------------------------------------------------------------

// ShaderCache::ShaderCache(GLint vertex, GLint frag)
// {
// 	const char* attrs[] = {
// 		"position", "normal", "tangent", "texcoord", NULL
// 	};
//
// 	program = link_program(vertex, frag, attrs);
//
// 	draw_params.world_uniform = glGetUniformLocation(program, "world_matrix");
// 	draw_params.norm_uniform  = glGetUniformLocation(program, "normal_matrix");
// 	draw_params.view_uniform  = glGetUniformLocation(program, "view_matrix");
// 	draw_params.proj_uniform  = glGetUniformLocation(program, "proj_matrix");
//
// 	draw_params.material_uniforms.tex  = glGetUniformLocation(program, "tex");
// 	draw_params.material_uniforms.norm = glGetUniformLocation(program, "norm");
// 	draw_params.material_uniforms.spec = glGetUniformLocation(program, "spec");
// 	draw_params.material_uniforms.envd = glGetUniformLocation(program, "envd");
// }