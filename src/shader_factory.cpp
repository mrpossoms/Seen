#include "shader.hpp"
#include "renderergl.hpp"
#include <iomanip>

using namespace seen;

std::string Shader::mat(int rank) { return "mat" + std::to_string(rank); }
//------------------------------------------------------------------------------

std::string Shader::vec(int rank)
{
	if (rank == 1)
	{
		return "float";
	}

	return "vec" + std::to_string(rank);
}
//------------------------------------------------------------------------------

std::string Shader::integer() { return "int"; }
//------------------------------------------------------------------------------

std::string Shader::shortint() { return "short"; }
//------------------------------------------------------------------------------

Shader& Shader::vertex(int feature_flags)
{
	if (feature_flags & Shader::VERT_POSITION)
	{
		input("position_in").as(Shader::vec(3));
	}

	if (feature_flags & Shader::VERT_NORMAL)
	{
		input("normal_in").as(Shader::vec(3));
	}

	if (feature_flags & Shader::VERT_TANGENT)
	{
		input("tangent_in").as(Shader::vec(3));
	}

	if (feature_flags & Shader::VERT_UV)
	{
		input("texcoord_in").as(Shader::vec(3));
	}

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::preceded_by(Shader& previous)
{
	for (auto output : previous.outputs)
	{
		Shader::Variable input(VAR_IN, output.type, output.name);
		input.array(output.array_size);
		inputs.push_back(input);
	}

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::transformed()
{
	Shader::Variable* pos  = has_input("position_*");
	Shader::Variable* norm = has_input("normal_*");
	Shader::Variable* tang = has_input("tangent_*");

	assert(pos);

	auto l_pos_trans = local("l_pos_trans").as(vec(4));
	auto u_world = parameter("u_world_matrix").as(mat(4));

	next(l_pos_trans = u_world * vec(4, "%s, 1.0", pos->cstr()));

	if (norm)
	{
		auto l_norm_rot = output("normal_" + suffix()).as(vec(3));
		auto u_normal_matrix = parameter("u_normal_matrix").as(mat(3));

		next(l_norm_rot = u_normal_matrix * *norm);
	}

	if (tang)
	{
		auto l_tang_rot = output("tangent_" + suffix()).as(vec(3));
		auto u_normal_matrix = parameter("u_normal_matrix").as(mat(3));

		next(l_tang_rot = u_normal_matrix * *tang);
	}

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::compute_binormal()
{
	auto o_binormal = output("biormal_" + suffix()).as(vec(3));

	Shader::Variable* norm = has_input("normal_*");
	Shader::Variable* tang = has_input("tangent_*");

	assert(norm && tang);

	next(o_binormal = norm->cross(*tang));

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::emit_position()
{
	assert(has_variable("l_pos_trans", locals));

	next(output("position_" + suffix()).as(vec(3)) = local("l_pos_trans")["xyz"]);

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::pass_through(std::string name)
{
	auto in = input(name);
	auto output_name = name.substr(0, name.find("_")) + "_" + suffix();

	next(output(output_name).as(in.type) = in);

	return *this;
}
//------------------------------------------------------------------------------

Shader&  Shader::viewed()
{
	assert(has_variable("l_pos_trans", locals));

	auto l_pos_trans = local("l_pos_trans").as(vec(4));
	auto l_pos_view = local("l_pos_view").as(vec(4));
	auto u_view = parameter("u_view_matrix").as(mat(4));

	next(l_pos_view = u_view * l_pos_trans);

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::projected()
{
	assert(has_variable("l_pos_view", locals));

	auto l_pos_view = local("l_pos_view").as(vec(4));
	auto l_pos_proj = local("l_pos_proj").as(vec(4));
	auto u_proj = parameter("u_proj_matrix").as(mat(4));

	next(l_pos_proj = u_proj * l_pos_view);

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::color_textured()
{
	auto u_color_sampler = parameter("u_color_sampler").as(tex(2));
	auto i_texcoord = input("texcoord_*").as(Shader::vec(3));
	auto color = output("color").as(Shader::vec(4));

	next(color = call("texture", {u_color_sampler, i_texcoord["xy"]}));

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::blinn()
{
	assert(has_input("normal_*"));

	Variable* normal = NULL;
	if (has_variable("l_normal", locals)) normal = has_variable("l_normal", locals);
	else if (has_variable("normal_*", inputs)) normal = has_variable("normal_*", inputs);

	auto i_position = input("position_*");
	auto o_color = output("color").as(Shader::vec(4));

	auto u_light_position = parameter("u_light_position").as(vec(3));
	auto u_light_diffuse = parameter("u_light_diffuse").as(vec(3));
	auto u_light_specular = parameter("u_light_specular").as(vec(3));
	auto u_light_ambient = parameter("u_light_ambient").as(vec(3));
	auto u_view_pos = parameter("u_view_position").as(vec(3));

	auto l_view_dir = local("l_view_dir").as(vec(3));
	auto l_light_color = local("l_light_color").as(vec(3));
	auto l_half = local("l_half").as(vec(3));
	auto l_intensity = local("l_intensity").as(vec(1));
	auto l_light_dir = local("l_light_dir").as(vec(3));
	auto l_light_dist = local("l_light_dist").as(vec(1));
	auto l_ndh = local("l_ndh").as(vec(1));

	next(l_view_dir = (u_view_pos - i_position).normalize());
	next(l_light_dist = call("distance", {i_position, u_light_position}));
	next(l_light_dir = (u_light_position - i_position).normalize());
	next(l_intensity = normal->dot(l_light_dir).saturate());
	next(l_half = (l_light_dir + l_view_dir).normalize());

	next(l_light_color = u_light_ambient);
	next(l_light_color += (l_intensity * u_light_diffuse) / l_light_dist);

	next(l_ndh = normal->dot(l_half));
	next(l_intensity = (l_ndh.saturate()).pow(16));
	next(l_light_color += (l_intensity * u_light_specular) / l_light_dist);
	next(o_color["rgb"] *= l_light_color);

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::normal_map()
{
	assert(has_input("normal_*"));
	assert(has_input("tangent_*"));
	assert(has_input("biormal_*"));
	assert(has_input("texcoord_*"));

	auto i_texcoord = input("texcoord_*");
	auto i_normal = input("normal_*");
	auto i_tangent = input("tangent_*");
	auto i_binormal = input("biormal_*");
	auto o_color = output("color").as(Shader::vec(4));

	auto u_normal_sampler = parameter("u_normal_sampler").as(tex(2));
	auto u_normal_matrix = parameter("u_normal_matrix").as(mat(3));

	auto l_normal = local("l_normal").as(vec(3));
	auto l_basis = local("l_basis").as(mat(3));
	auto l_norm_sample = local("l_norm_sample").as(vec(3));

	next(l_basis = mat3(i_tangent, i_binormal, i_normal));
	next(l_norm_sample = call("texture", { u_normal_sampler, i_texcoord["xy"]})["xyz"] * 2.0 - 1.0);
	next(l_normal = l_basis * l_norm_sample);

	return *this;
}
//------------------------------------------------------------------------------

Shader::Shader(std::string name, GLenum type)
{
	this->name = name;
	this->type = type;
}
//------------------------------------------------------------------------------

Shader::~Shader()
{

}
//------------------------------------------------------------------------------

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

	static Shader::Variable empty(VAR_NONE, "", "");
	return empty;
}
//------------------------------------------------------------------------------

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
//------------------------------------------------------------------------------

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

	for (auto local : locals)
	{
		src << "\t" << local.declaration() << ";" << std::endl;
	}
	src << std::endl;

	for (auto statement : statements)
	{
		src << "\t" << statement.str << ";" << std::endl;
	}
	src << "}" << std::endl;

	return src.str();
}
//------------------------------------------------------------------------------

GLint Shader::compile()
{
	GLint shader = compile_source(code().c_str(), type);
	Shaders._shader_cache[name] = shader;

	return shader;
}
//------------------------------------------------------------------------------

Shader Shader::vertex(std::string name)
{
	Shader shader(name, GL_VERTEX_SHADER);
	return shader;
}
//------------------------------------------------------------------------------

Shader Shader::tessalation_control(std::string name)
{
	Shader shader(name, GL_TESS_CONTROL_SHADER);
	return shader;
}
//------------------------------------------------------------------------------

Shader Shader::tessalation_evaluation(std::string name)
{
	Shader shader(name, GL_TESS_EVALUATION_SHADER);
	return shader;
}
//------------------------------------------------------------------------------

Shader Shader::geometry(std::string name)
{
	Shader shader(name, GL_GEOMETRY_SHADER);
	return shader;
}
//------------------------------------------------------------------------------

Shader Shader::fragment(std::string name)
{
	Shader shader(name, GL_FRAGMENT_SHADER);
	return shader;
}


std::string Shader::suffix()
{
	std::map<GLint, std::string> suffixes = {
		{ GL_VERTEX_SHADER,          "vsh" },
		{ GL_TESS_CONTROL_SHADER,    "tcs" },
		{ GL_TESS_EVALUATION_SHADER, "tes" },
		{ GL_GEOMETRY_SHADER,        "geo" },
		{ GL_FRAGMENT_SHADER,        "fsh" },
	};

	return suffixes[type];
}
