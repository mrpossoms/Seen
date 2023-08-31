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

std::string Shader::cubemap() { return "samplerCube"; }
//------------------------------------------------------------------------------
std::string Shader::shadowCube() { return "samplerCubeShadow"; }
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

Shader::Variable& Shader::input(std::string name)
{
	Shader::Variable* input = has_variable(name, inputs);

	if (input) return *input;

	Shader::Variable var = { Shader::VarRole::VAR_IN, "", name };
	inputs.push_back(var);

	return inputs[inputs.size() - 1];
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::output(std::string name)
{
	Shader::Variable* output = has_variable(name, outputs);

	if (output) return *output;

	Shader::Variable var = { Shader::VarRole::VAR_OUT, "", name };
	outputs.push_back(var);

	return outputs[outputs.size() - 1];
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::parameter(std::string name)
{
	Shader::Variable* parameter = has_variable(name, parameters);

	if (parameter) return *parameter;

	Shader::Variable var = { Shader::VarRole::VAR_PARAM, "", name };
	parameters.push_back(var);

	return parameters[parameters.size() - 1];
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::local(std::string name)
{
	Shader::Variable* local = has_variable(name, locals);

	if (local) return *local;

	Shader::Variable var = { Shader::VarRole::VAR_LOCAL, "", name };
	locals.push_back(var);

	return locals[locals.size() - 1];
}
//------------------------------------------------------------------------------

Shader::Variable* Shader::has_variable(std::string name, std::vector<Variable>& vars)
{
	bool wild = false;

	if (name[0] == '*')
	{
		wild = true;
		name = name.substr(1);
	}
	else if (name[name.length() - 1] == '*')
	{
		wild = true;
		name = name.substr(0, name.length() - 1);
	}

	for (int i = vars.size(); i--;)
	{
		if (wild)
		{
			if (vars[i].name.find(name) != std::string::npos)
			{
				return &vars[i];
			}
		}
		else if (vars[i].name == name && vars[i].name.length() == name.length())
		{
			return &vars[i];
		}
	}

	return nullptr;
}
//------------------------------------------------------------------------------

Shader::Variable* Shader::has_input(std::string name)
{
	return has_variable(name, inputs);
}
//------------------------------------------------------------------------------

Shader::Variable* Shader::has_output(std::string name)
{
	return has_variable(name, outputs);
}

//------------------------------------------------------------------------------

Shader::Expression Shader::vec2(float x, float y)
{
	return call("vec2", {
		{std::to_string(x)}, {std::to_string(y)}
	});
}
//------------------------------------------------------------------------------

Shader::Expression Shader::vec<3>(float x, float y, float z)
{
	return call("vec<3>", {
		{std::to_string(x)}, {std::to_string(y)}, {std::to_string(z)}
	});
}
//------------------------------------------------------------------------------

Shader::Expression Shader::vec4(float x, float y, float z, float w)
{
	return call("vec4", {
		{std::to_string(x)}, {std::to_string(y)}, {std::to_string(z)}, {std::to_string(w)}
	});
}
//------------------------------------------------------------------------------

Shader::Expression Shader::mat3(Shader::Expression c0, Shader::Expression c1, Shader::Expression c2)
{
	return call("mat3", {
		c0, c1, c2
	});
}

//------------------------------------------------------------------------------

Shader& Shader::preceded_by(Shader& previous)
{
	for (auto output : previous.outputs)
	{
		Shader::Variable input(Shader::VarRole::VAR_IN, output.type, output.name);
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

	next(output("position_" + suffix()).as(vec(4)) = local("l_pos_trans"));

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

Shader& Shader::color_white()
{
	auto color = output("color").as(Shader::vec(4));
	next(color = vec4(1, 1, 1, 1));

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

	auto l_normal = local("l_normal").as(vec(3));
	auto u_specular_sampler = parameter("u_specular_sampler").as(tex(2));

	if (has_variable("l_normal", locals) == nullptr && has_variable("normal_*", inputs))
	{
		Variable::copy(*has_variable("normal_*", inputs), l_normal);
	}

	auto i_position = input("position_*");
	auto i_texcoord = input("texcoord_*").as(Shader::vec(3));
	auto o_color = output("color").as(Shader::vec(4));

	auto u_light_position = parameter("u_light_position").as(vec(3));
	auto u_light_power = parameter("u_light_power").as(vec(3));
	auto u_view_pos = parameter("u_view_position").as(vec(3));
	auto u_light_ambience = parameter("u_light_ambience").as(vec(1));

	auto l_view_dir = local("l_view_dir").as(vec(3));
	auto l_light_color = local("l_light_color").as(vec(3));
	auto l_half = local("l_half").as(vec(3));
	auto l_intensity = local("l_intensity").as(vec(1));
	auto l_light_dir = local("l_light_dir").as(vec(3));
	auto l_light_dist = local("l_light_dist").as(vec(1));
	auto l_ndh = local("l_ndh").as(vec(1));

	next(l_view_dir = (u_view_pos - i_position["xyz"]).normalize());
	next(l_light_dir = (u_light_position - i_position["xyz"]));
	next(l_light_dist = l_light_dir.length());
	next(l_light_dir = l_light_dir.normalize());
	next(l_intensity = l_normal.dot(l_light_dir).saturate());

	if (has_variable("l_lit", locals))
	{
		next(l_intensity *= local("l_lit"));
	}

	next(l_half = (l_light_dir + l_view_dir).normalize());

	next(l_light_color = u_light_power * u_light_ambience);
	next(l_light_color += (l_intensity * u_light_power) / l_light_dist);


	next(l_ndh = l_normal.dot(l_half));
	next(l_intensity = (l_ndh.saturate()).pow(16));
	next(l_intensity *= call("texture", {u_specular_sampler, i_texcoord["xy"]})["r"]);

	if (has_variable("l_lit", locals))
	{
		next(l_intensity *= local("l_lit"));
	}

	next(l_light_color += (l_intensity * u_light_power) / l_light_dist);
	next(o_color["rgb"] *= l_light_color);

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::normal_mapped()
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
	next(l_normal = l_basis * l_norm_sample * -1.f);

	next_if(builtin("gl_FrontFacing"), [&]{
		next(l_normal *= -1.f);
	});

	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::shadow_mapped(bool for_point_light)
{
	assert(has_input("position_*"));

	auto position = input("position_*");
	auto l_projected = local("l_projected").as(vec(4));
	auto l_calculated_dist = local("l_calc_dist").as(vec(1));
	auto l_sampled_dist = local("l_samp_dist").as(vec(1));
	auto l_lit = local("l_lit").as(vec(1));
	auto l_light_dir = local("l_light_dir").as(vec(3));

	auto u_shadow_cube = parameter("u_shadow_cube").as(cubemap());
	auto u_light_position = parameter("u_light_position").as(vec(3));

	next(l_lit = 1.0);
	next(l_light_dir = position["xyz"] - u_light_position );
	next(l_calculated_dist = l_light_dir.length() / 100.f);
	next(l_sampled_dist = call("texture", { u_shadow_cube, l_light_dir })["r"]);

	next_if((l_calculated_dist - l_sampled_dist) > 0.001, [&]{
		next(l_lit = 0.5);
	});


	return *this;
}
//------------------------------------------------------------------------------

Shader& Shader::shadow_mapped_vsm(bool for_point_light)
{
	assert(has_input("position_*"));

	auto i_position = input("position_*");

	auto l_projected = local("l_projected").as(vec(4));
	auto l_depth = local("l_depth").as(vec(4));
	auto l_lit = local("l_lit").as(vec(1));
	auto l_light_dir = local("l_light_dir").as(vec(3));

	auto l_query = local("l_query").as(vec(2));
	auto l_E_x2 = local("l_E_x2").as(vec(1));
	auto l_Ex_2 = local("l_Ex_2").as(vec(1));
	auto l_var = local("l_var").as(vec(1));
	auto l_md = local("l_md").as(vec(1));
	auto l_md_2 = local("l_mD_2").as(vec(1));
	auto l_p = local("l_p").as(vec(1));

	auto u_shadow_cube = parameter("u_shadow_cube").as(cubemap());
	auto u_light_position = parameter("u_light_position").as(vec(3));
	auto u_light_projection = parameter("u_light_proj_matrix").as(mat(4));

	seen::Shader::Expression one = { "1.f" };

	next(l_lit = 1.0);
	next(l_light_dir = i_position["xyz"] - u_light_position );
	next(l_query = call("texture", { u_shadow_cube, l_light_dir})["rg"] * 1000.f);

	next(l_depth = call("vec4", {{"0"}, {"0"}, l_light_dir.length(), {"1.0"}}));
	//next(l_depth /= 1000.f);
	next(l_E_x2 = l_query["g"]);
	next(l_Ex_2 = l_query["r"].pow(2));
	next(l_var = l_E_x2 - l_Ex_2);
	next(l_md = l_query["r"] - l_depth["z"]);
	next(l_md_2 = l_md.pow(2));
	next(l_p = l_var + l_md_2);
	next(l_p = l_var / l_p);

	next_if(l_depth["z"] > l_query["r"], [&]{
		next(l_lit = l_p);
	});

	return *this;
}
//------------------------------------------------------------------------------

Shader::Shader(std::string name, GLenum type)
{
	this->name = name;
	this->type = type;

	_code_block = 1;
}
//------------------------------------------------------------------------------

Shader::~Shader()
{

}
//------------------------------------------------------------------------------

Shader::Expression Shader::builtin(std::string gl_name)
{
	using VRole = Shader::VarRole;

	static std::map<std::string, Shader::Variable> vertex_builtin_map = {
		{ "gl_VertexID",     Shader::Variable(VRole::VAR_IN, "int", "gl_VertexID") },
		{ "gl_InstanceID",   Shader::Variable(VRole::VAR_IN, "int", "gl_InstanceID") },
		{ "gl_DrawID",       Shader::Variable(VRole::VAR_IN, "int", "gl_DrawID") },
		{ "gl_BaseVertex",   Shader::Variable(VRole::VAR_IN, "int", "gl_BaseVertex") },
		{ "gl_BaseInstance", Shader::Variable(VRole::VAR_IN, "int", "gl_BaseInstance") },

		{ "gl_Position",     Shader::Variable(VRole::VAR_OUT, "vec4", "gl_Position") },
		{ "gl_PointSize",    Shader::Variable(VRole::VAR_OUT, "float", "gl_PointSize") },
	};

	static std::map<std::string, Shader::Variable> tess_control_builtin_map = {
		{ "gl_PatchVerticesIn", Shader::Variable(VRole::VAR_IN, "int", "gl_PatchVerticesIn") },
		{ "gl_PrimitiveID",     Shader::Variable(VRole::VAR_IN, "int", "gl_PrimitiveID") },
		{ "gl_InvocationID",    Shader::Variable(VRole::VAR_IN, "int", "gl_InvocationID") },
		{ "gl_in", Shader::Variable(VRole::VAR_IN, "struct", "gl_in").array(32)
		                                                       << Shader::Variable(VRole::VAR_IN, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VRole::VAR_IN, "float", "gl_PointSize")
		                                                       << Shader::Variable(VRole::VAR_IN, "float", "gl_ClipDistance").array(32)},

		{ "gl_TessLevelOuter",     Shader::Variable(VRole::VAR_OUT, "float", "gl_TessLevelOuter").array(4) },
		{ "gl_TessLevelInner",    Shader::Variable(VRole::VAR_OUT, "float", "gl_TessLevelInner").array(2) },
		{ "gl_out", Shader::Variable(VRole::VAR_OUT, "struct", "gl_out").array(32)
		                                                       << Shader::Variable(VRole::VAR_OUT, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VRole::VAR_OUT, "float", "gl_PointSize")
		                                                       << Shader::Variable(VRole::VAR_OUT, "float", "gl_ClipDistance").array(32)},
	};

	static std::map<std::string, Shader::Variable> tess_eval_builtin_map = {
		{ "gl_TessCoord", Shader::Variable(VRole::VAR_IN, "vec<3>", "gl_TessCoord") },
		{ "gl_PatchVerticesIn",     Shader::Variable(VRole::VAR_IN, "int", "gl_PatchVerticesIn") },
		{ "gl_PrimitiveID",    Shader::Variable(VRole::VAR_IN, "int", "gl_PrimitiveID") },
		{ "gl_TessLevelOuter",     Shader::Variable(VRole::VAR_IN, "float", "gl_TessLevelOuter").array(4) },
		{ "gl_TessLevelInner",    Shader::Variable(VRole::VAR_IN, "float", "gl_TessLevelInner").array(2) },
		{ "gl_in", Shader::Variable(VRole::VAR_IN, "struct", "gl_in").array(32)
		                                                       << Shader::Variable(VRole::VAR_IN, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VRole::VAR_IN, "float", "gl_PointSize")
		                                                       << Shader::Variable(VRole::VAR_IN, "float", "gl_ClipDistance").array(32)},

		{ "gl_Position",     Shader::Variable(VRole::VAR_OUT, "vec4", "gl_Position") },
		{ "gl_PointSize",    Shader::Variable(VRole::VAR_OUT, "float", "gl_PointSize") },
		{ "gl_ClipDistance", Shader::Variable(VRole::VAR_OUT, "float", "gl_ClipDistance").array(32) },
	};

	static std::map<std::string, Shader::Variable> geometry_builtin_map = {
		{ "gl_in", Shader::Variable(VRole::VAR_IN, "struct", "gl_in").array(32)
		                                                       << Shader::Variable(VRole::VAR_IN, "vec4", "gl_Position")
	 	                                                       << Shader::Variable(VRole::VAR_IN, "float", "gl_PointSize")
		                                                       << Shader::Variable(VRole::VAR_IN, "float", "gl_ClipDistance").array(32)},
		{ "gl_PrimitiveIDIn",     Shader::Variable(VRole::VAR_IN, "int", "gl_PrimitiveIDIn") },
		{ "gl_InvocationID",    Shader::Variable(VRole::VAR_IN, "int", "gl_InvocationID") },

		{ "gl_Position",     Shader::Variable(VRole::VAR_OUT, "vec4", "gl_Position") },
		{ "gl_PointSize",    Shader::Variable(VRole::VAR_OUT, "float", "gl_PointSize") },
		{ "gl_ClipDistance", Shader::Variable(VRole::VAR_OUT, "float", "gl_ClipDistance").array(32) },
	};

	static std::map<std::string, Shader::Variable> fragment_builtin_map = {
		{ "gl_FragCoord",     Shader::Variable(VRole::VAR_IN, "vec4", "gl_FragCoord") },
		{ "gl_FrontFacing",   Shader::Variable(VRole::VAR_IN, "bool", "gl_FrontFacing") },
		{ "gl_PointCoord",    Shader::Variable(VRole::VAR_IN, "vec2", "gl_PointCoord") },
		{ "gl_SampleID",      Shader::Variable(VRole::VAR_IN, "int", "gl_SampleID") },
		{ "gl_PointCoord",    Shader::Variable(VRole::VAR_IN, "vec2", "gl_PointCoord") },
		{ "gl_SampleMaskIn",  Shader::Variable(VRole::VAR_IN, "int", "gl_SampleMaskIn") },
		{ "gl_ClipDistance",  Shader::Variable(VRole::VAR_IN, "float", "gl_ClipDistance").array(32) },
		{ "gl_PrimitiveID",   Shader::Variable(VRole::VAR_IN, "int", "gl_PrimitiveID") },
		{ "gl_Layer",         Shader::Variable(VRole::VAR_IN, "int", "gl_Layer") },
		{ "gl_ViewportIndex", Shader::Variable(VRole::VAR_IN, "int", "gl_ViewportIndex") },

		{ "gl_FragDepth",     Shader::Variable(VRole::VAR_OUT, "float", "gl_FragDepth") },
		{ "gl_Color",         Shader::Variable(VRole::VAR_OUT, "vec4", "gl_Color") },
		{ "gl_SampleMask",    Shader::Variable(VRole::VAR_OUT, "int", "gl_SampleMask").array(32) },
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

	static Shader::Variable empty(VRole::VAR_NONE, "", "");
	return empty;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::call(std::string func_name, std::vector<Expression> params)
{
	Expression call = { func_name + "(" };

	for (unsigned int i = 0; i < params.size(); i++)
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
		for (unsigned int i = 0; i < vars.size(); i++)
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
		src << statement.str; // << ";" << std::endl;
		const char c = statement.str[statement.str.length() - 1];
		if (c != '{' && c != '}')
		{
			src << ";";
		}

		src << std::endl;
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
