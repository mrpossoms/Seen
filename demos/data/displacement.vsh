#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 texcoord;

out vec2 v_texcoord; // texture coords
out vec3 v_normal;   // normal
out vec3 v_tangent;
out vec3 v_binormal; // binormal (for TBN basis calc)
out vec3 v_pos;      // pixel view space position
out vec4 v_screen_space;

uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat3 normal_matrix;
uniform mat4 world_matrix;

uniform sampler2D u_displacement;     // base texture (albedo)

uniform float u_texcoord_rotation;

void main()
{
	vec3 binormal = cross(normal, tangent);

	float t = u_texcoord_rotation;
	mat2 uv_rot = mat2(
		vec2(cos(t), sin(t)),
		vec2(-sin(t), cos(t))
	);

	v_texcoord = uv_rot * ((texcoord.xy - 0.5) * 2.0);
	v_normal   = normalize(normal_matrix * normal);
	v_tangent  = normalize(normal_matrix * tangent);
	v_binormal = cross(v_normal, v_tangent);

	vec3 displacement = 0.25 * normal * (texture(u_displacement, v_texcoord).x - 0.8);
	vec4 world_space = world_matrix * vec4(position + displacement, 1.0);
	vec4 view_space = view_matrix * world_space;
	gl_Position = v_screen_space = proj_matrix * view_space;

	v_pos = normalize(normal_matrix * position); //view_space.xyz;
}
