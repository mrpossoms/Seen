#version 330 core

in vec2 g_texcoord; // texture coords
in vec3 g_normal;   // normal
in vec3 g_tangent;  // tangent
in vec3 g_binormal; // binormal (for TBN basis calc)

out vec4 color;

uniform sampler2D us_color;     // base texture (albedo)
uniform sampler2D us_normal;
uniform sampler2D us_displacement;

uniform vec3 u_light_dir;
uniform float u_green;

void main()
{
    vec3 rgb = texture(us_color, g_texcoord).xyz;// * vec3(u_green, 1.0, 1.0);
    vec3 tn = normalize((texture(us_normal, g_texcoord).xyz * 2.0) - 1.0);
    mat3 tbn = mat3(g_binormal, g_tangent, g_normal);

    vec3 normal = tbn * tn;
    //vec3 normal = tbn * vec3(0.0, 0.0, 1.0);

    float shade = (dot(normalize(-u_light_dir), normal) + 1.0) / 2.0;
    float l = shade * 0.75 +  0.25;
	// color = vec4(1.0, 0.0, 0.0, 1.0);
    // color = vec4(vec3(v_texcoord, 1.0), 1.0);
    // color = vec4(rgb.xyz * l, 1.0);
    // color = vec4(l * ((g_normal.xyz / 2.0) + 0.5), 1.0);
    color = vec4(((g_normal.xyz / 2.0) + 0.5), 1.0);
}
