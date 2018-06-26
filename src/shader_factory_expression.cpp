#include "shader.hpp"
#include "renderergl.hpp"
#include <iomanip>

using namespace seen;


Shader::Expression::Expression(std::string s)
{
	str = s;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::mat(int rank, const char* fmt, ...)
{
	char args[128];

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(args, sizeof(args), fmt, ap);
	va_end(ap);

	return { Shader::mat(rank) + "(" + std::string(args) + ")" };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::vec(int rank, const char* fmt, ...)
{
	char args[128];

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(args, sizeof(args), fmt, ap);
	va_end(ap);

	return { Shader::vec(rank) + "(" + std::string(args) + ")" };
}
//------------------------------------------------------------------------------

std::string Shader::tex(int rank)
{
	switch(rank)
	{
		case 2:
			return "sampler2D";
		case 3:
			return "samplerCube";
		default:
			return "";
	}
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator+ (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " + " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator+= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " += " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator- (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " - " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator-= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " -= " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator* (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " * " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator*= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " *= " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator/ (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " / " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator/= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " /= " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " = " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator== (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " == " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator< (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " < " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator> (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " > " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator<= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " <= " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator>= (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " >= " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator<< (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " << " + e.str };
	return eo;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator>> (Shader::Expression e)
{
	Shader::Expression eo = { this->str + " >> " + e.str };
	return eo;
}
//------------------------------------------------------------------------------


Shader::Expression Shader::Expression::operator+ (std::string e) { return *this + Shader::Expression(e); }
Shader::Expression Shader::Expression::operator+= (std::string e) { return *this += Shader::Expression(e); }
Shader::Expression Shader::Expression::operator- (std::string e) { return *this - Shader::Expression(e); }
Shader::Expression Shader::Expression::operator-= (std::string e) { return *this -= Shader::Expression(e); }
Shader::Expression Shader::Expression::operator* (std::string e) { return *this * Shader::Expression(e); }
Shader::Expression Shader::Expression::operator*= (std::string e) { return *this *= Shader::Expression(e); }
Shader::Expression Shader::Expression::operator/ (std::string e) { return *this / Shader::Expression(e); }
Shader::Expression Shader::Expression::operator/= (std::string e) { return *this /= Shader::Expression(e); }
Shader::Expression Shader::Expression::operator= (std::string e) { return *this = Shader::Expression(e); }

Shader::Expression Shader::Expression::operator+ (float e) { return *this + Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator+= (float e) { return *this += Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator- (float e)  { return *this - Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator-= (float e)  { return *this -= Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator* (float e)  { return *this * Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator*= (float e)  { return *this *= Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator/ (float e)  { return *this / Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator/= (float e)  { return *this /= Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator= (float e)  { return *this = Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator< (float e)  { return *this < Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator> (float e)  { return *this > Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator<= (float e)  { return *this <= Shader::Expression(std::to_string(e)); }
Shader::Expression Shader::Expression::operator>= (float e)  { return *this >= Shader::Expression(std::to_string(e)); }

Shader::Expression Shader::Expression::normalize()
{
	return { "normalize(" + str + ")" };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::dot(Shader::Expression e)
{
	return { "dot(" + this->str + ", " + e.str + ")" };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::cross(Shader::Expression e)
{
	return { "cross(" + this->str + ", " + e.str + ")" };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::pow(float power)
{
	return { "pow(" + this->str + ", " + std::to_string(power) + ")" };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::mix(std::vector<Shader::Expression> params, float percent)
{
	Shader::Expression p = { std::to_string(percent) };
	return mix(params, p);
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::mix(std::vector<Shader::Expression> params, Shader::Expression percent)
{
	std::string exp = "mix(";

	params.insert(params.begin(), this->str);

	for (auto p : params)
	{
		exp += p.str + ", ";
	}

	return { exp + percent.str + ")" };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::saturate()
{
	return { "clamp(" + str + ", 0.0, 1.0)" };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Expression::operator[] (std::string swizzel)
{
	Shader::Expression eo = { this->str + "." + swizzel };
	return eo;
}
//------------------------------------------------------------------------------

const char* Shader::Expression::cstr() { return str.c_str(); }
//------------------------------------------------------------------------------

Shader::Variable::Variable(VarRole role, std::string type, std::string name) : Expression(name)
{
	this->role = role;
	this->type = type;
	this->name = name;
	this->str = name;
	this->array_size = 0;
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::Variable::as(std::string type)
{
	this->type = type;
	return *this;
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::Variable::array(int dims)
{
	array_size = dims;
	return *this;
}
//------------------------------------------------------------------------------

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
		case VAR_OUT:
			return "out " + this->type + " " + this->name + rank;
		case VAR_INOUT:
			return "inout " + this->type + " " + this->name + rank;
		case VAR_PARAM:
			return "uniform " + this->type + " " + this->name + rank;
		case VAR_LOCAL:
			return  this->type + " " + this->name + rank;
		case VAR_NONE:
			return "";
	}
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Variable::at_index(int i)
{
	assert(array_size > 0);

	return { str + "[" + std::to_string(i) + "]" };
}
//------------------------------------------------------------------------------

Shader& Shader::next(Shader::Expression e)
{
	statements.push_back(e);
	return *this;
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::input(std::string name)
{
	Shader::Variable* input = has_variable(name, inputs);

	if (input) return *input;

	Shader::Variable var = { VAR_IN, "", name };
	inputs.push_back(var);

	return inputs[inputs.size() - 1];
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::output(std::string name)
{
	Shader::Variable* output = has_variable(name, outputs);

	if (output) return *output;

	Shader::Variable var = { VAR_OUT, "", name };
	outputs.push_back(var);

	return outputs[outputs.size() - 1];
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::parameter(std::string name)
{
	Shader::Variable* parameter = has_variable(name, parameters);

	if (parameter) return *parameter;

	Shader::Variable var = { VAR_PARAM, "", name };
	parameters.push_back(var);

	return parameters[parameters.size() - 1];
}
//------------------------------------------------------------------------------

Shader::Variable& Shader::local(std::string name)
{
	Shader::Variable* local = has_variable(name, locals);

	if (local) return *local;

	Shader::Variable var = { VAR_LOCAL, "", name };
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
		else if (vars[i].name == name)
		{
			return &vars[i];
		}
	}

	return NULL;
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

Shader::Expression Shader::vec3(float x, float y, float z)
{
	return call("vec3", {
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


Shader::Variable& Shader::Variable::operator<< (Shader::Variable property)
{
	if (type == "struct")
	{
		properties[property.name] = property;
	}

	return *this;
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Variable::operator[] (std::string lookup)
{
	if (type == "struct")
	{
		return properties[lookup];
	}

	return { str + "." + lookup };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Variable::operator= (Shader::Expression e)
{
	return { str + " = " + e.str };
}
//------------------------------------------------------------------------------

Shader::Expression Shader::Variable::operator= (Shader::Variable e)
{
	return { str + " = " + e.str };
}
