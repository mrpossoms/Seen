#include "shader.hpp"
#include "renderergl.hpp"
#include <iomanip>

seen::ShaderCache seen::Shaders;

using namespace seen;


GLint compile_source(const char* src, GLenum type)
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

		std::cerr << SEEN_TERM_YELLOW << src << SEEN_TERM_COLOR_OFF << std::endl;
		exit(-2);
	}

	assert(gl_get_error());
	std::cerr << SEEN_TERM_GREEN "OK" SEEN_TERM_COLOR_OFF << std::endl;

	return shader;
}


GLint load_shader(const char* path, GLenum type)
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

GLint link_program(const GLint* shaders, const char** attributes)
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
	char attribute_store[16][128] = {};
	char* attributes[16] = {};
	GLint gs_shaders[6] = {};
	ShaderProgram program;
	std::string prog_name;

	// compile, cache and include each shader
	int si = 0;
	for (auto shader : shaders)
	{
		if (shader.type == GL_VERTEX_SHADER)
		{

			for (unsigned int i = 0; i < shader.inputs.size(); i++)
			{
				attributes[i] = attribute_store[i];
				strncpy(attributes[i], shader.inputs[i].name.c_str(), sizeof(attribute_store[i]));
			}
		}

		gs_shaders[si] = shader.compile();
		si++;
		prog_name += shader.name;
	}

	program.program = link_program(gs_shaders, (const char**)attributes);
	program.primative = GL_TRIANGLES;
	Shaders._program_cache[prog_name] = program;

	ShaderProgram& prog_ref = Shaders._program_cache[prog_name];

	// preload the uniforms
	for (auto shader : shaders)
	{
		for (auto param : shader.parameters)
		{
			prog_ref[param.name];
		}
	}

	return &Shaders._program_cache[prog_name];
}
//------------------------------------------------------------------------------


ShaderProgram* ShaderProgram::builtin_sky()
{
	const std::string prog_name = "sky_vshsky_fsh";

	if (Shaders._program_cache.count(prog_name) == 1)
	{
		return &Shaders._program_cache[prog_name];
	}

	auto vsh = Shader::vertex("sky_vsh");
	auto fsh = Shader::fragment("sky_fsh");

	vsh.vertex(seen::Shader::VERT_POSITION | seen::Shader::VERT_NORMAL | seen::Shader::VERT_TANGENT | seen::Shader::VERT_UV);
	vsh.transformed()
	   .next(vsh.local("l_pos_trans")["xyz"] *= "100.0")
	   .viewed()
	   .projected()
	   .next(vsh.builtin("gl_Position") = vsh.local("l_pos_proj"))
	   .compute_binormal()
	   .pass_through("texcoord_in");

	vsh.next(vsh.output("view_position_" + vsh.suffix()).as(Shader::vec(3)) = vsh.local("l_pos_trans")["xyz"]);


	fsh.preceded_by(vsh);
	auto color = fsh.output("color_" + fsh.suffix()).as(Shader::vec(4));

	auto light_blue = fsh.local("light_blue").as(Shader::vec(3));
	auto dark_blue = fsh.local("dark_blue").as(Shader::vec(3));
	auto sun_color = fsh.local("sun_color").as(Shader::vec(3));

	auto left_forward = fsh.local("left_forward").as(Shader::vec(3));
	auto light_dir = fsh.local("light_dir").as(Shader::vec(3));
	auto norm = fsh.local("norm").as(Shader::vec(3));
	auto horizon = fsh.local("horizon").as(Shader::vec(1));
	auto haze = fsh.local("haze").as(Shader::vec(1));
	auto blue = fsh.local("blue").as(Shader::vec(3));
	auto sun = fsh.local("sun").as(Shader::vec(3));
	auto light_angle = fsh.local("light_angle").as(Shader::vec(1));

	fsh.next(left_forward = fsh.vec3(1.0, 0.0, 1.0))
	   .next(light_dir = fsh.vec3(0.0, 1.0, 1.0))
	   .next(dark_blue = fsh.vec3(4.0 / 255.0, 39.0 / 255.0, 181.0 / 255.0))
	   .next(light_blue = fsh.vec3(131.0 / 255.0, 187.0 / 255.0, 248.0 / 255.0))
	   .next(sun_color = fsh.vec3(253.0 / 255.0, 184.0 / 255.0, 19.0 / 255.0))

	   .next(norm = fsh.input("view_position_*").normalize())
	   .next(light_angle = (light_dir.normalize().dot(norm) + 1.0))
	   .next(light_angle /= 2.0)

	   .next(horizon = norm.dot((norm * left_forward).normalize()))
	   .next(haze = ((light_dir.dot(norm) + horizon) + 1.0) / 2.0)
	   .next(blue = dark_blue.mix({light_blue}, horizon.pow(2.0)))
	   .next(sun = sun_color * (light_angle.pow(64) * 10.0))

	   .next(color = fsh.call("vec4", { blue.mix({sun}, light_angle.pow(64)), {"1.0"}}));


	return seen::ShaderProgram::compile({ vsh, fsh });
}
//------------------------------------------------------------------------------

ShaderProgram* ShaderProgram::builtin_normal_colors()
{
	const std::string prog_name = "default_vshnormal_color_fsh";

	if (Shaders._program_cache.count(prog_name) == 1)
	{
		return &Shaders._program_cache[prog_name];
	}

	auto vsh = Shader::vertex("default_vsh");
	auto fsh = Shader::fragment("normal_color_fsh");

	vsh.vertex(seen::Shader::VERT_POSITION | seen::Shader::VERT_NORMAL | seen::Shader::VERT_TANGENT | seen::Shader::VERT_UV);
	vsh.transformed()
	   .viewed()
	   .projected()
	   .next(vsh.builtin("gl_Position") = vsh.local("l_pos_proj"))
	   .pass_through("texcoord_in");

	fsh.preceded_by(vsh);
	auto color = fsh.output("color_" + fsh.suffix()).as(Shader::vec(4));

	fsh.next(color["rgb"] = fsh.input("normal_*") / 2.0 + 0.5);
	fsh.next(color["a"] = "1.0");

	return seen::ShaderProgram::compile({ vsh, fsh });
}
//------------------------------------------------------------------------------

ShaderProgram* ShaderProgram::builtin_shadow_depth()
{
	const std::string prog_name = "default_vshshadow_depth_fsh";

	if (Shaders._program_cache.count(prog_name) == 1)
	{
		return &Shaders._program_cache[prog_name];
	}

	auto vsh = Shader::vertex("default_vsh");
	auto fsh = Shader::fragment("shadow_depth_fsh");

	vsh.vertex(seen::Shader::VERT_POSITION | seen::Shader::VERT_NORMAL | seen::Shader::VERT_TANGENT | seen::Shader::VERT_UV);
	vsh.transformed()
	   .viewed()
	   .projected()
	   .next(vsh.output("position_" + vsh.suffix()).as(Shader::vec(3)) = vsh.local("l_pos_view")["xyz"])
	   .next(vsh.builtin("gl_Position") = vsh.local("l_pos_proj"));

	fsh.preceded_by(vsh);
	auto depth = fsh.output("depth_" + fsh.suffix()).as(Shader::vec(4));

	//fsh.next(depth = fsh.builtin("gl_FragCoord")["xyz"].length());
	fsh.next(depth["r"] = fsh.input("position_*").length());
	fsh.next(depth["g"] = depth["r"] * depth["r"]);

	return seen::ShaderProgram::compile({ vsh, fsh });
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

	for (int i = 3; i--;)
	{
		glActiveTexture(GL_TEXTURE0 + _tex_counter);
		glBindTexture(GL_TEXTURE_2D, m->v[i]);
		(*this)[uniform_names[i]] << _tex_counter;
		_tex_counter++;
	}
}
//------------------------------------------------------------------------------

void ShaderProgram::operator<<(Viewer* v)
{
	(*this)["u_view_matrix"] << v->_view;
	(*this)["u_proj_matrix"] << v->_projection;
}
//------------------------------------------------------------------------------

void ShaderProgram::operator<<(Positionable* p)
{
	(*this)["u_world_matrix"] << p->world();
	(*this)["u_normal_matrix"] << p->normal_matrix;
}
//------------------------------------------------------------------------------

void ShaderProgram::operator<<(Light* l)
{
	(*this)["u_light_position"] << l->position;
	(*this)["u_light_power"] << l->power;
	(*this)["u_light_ambience"] << l->ambience;
}
//------------------------------------------------------------------------------

void ShaderProgram::operator<<(ShadowPass* s)
{
	(*this)["u_shadow_cube"] << s->_cubemap;
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

void ShaderParam::operator<<(Vec3 v)
{
	glUniform3fv(_uniform, 1, (GLfloat*)v.v);
}
//------------------------------------------------------------------------------
//
// void ShaderParam::operator<<(Vec4 v)
// {
// 	glUniform4fv(_uniform, 1, (GLfloat*)v.v);
// }
// //------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------

void ShaderParam::operator<<(Cubemap* c)
{
	//return; // TODO
	glActiveTexture(GL_TEXTURE0 + _program->_tex_counter);
	assert(gl_get_error());
	glBindTexture(GL_TEXTURE_CUBE_MAP, c->_map);
	assert(gl_get_error());
	glUniform1i(_uniform, _program->_tex_counter);
	assert(gl_get_error());
	_program->_tex_counter++;
}
