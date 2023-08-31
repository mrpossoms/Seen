#version 400 core

in vec2 te_texcoord; // texture coords
in vec<3> te_normal;   // normal
in vec<3> te_tangent;  // tangent
in vec<3> te_binormal; // binormal (for TBN basis calc)

out vec4 color;

uniform sampler2D us_color;     // base texture (albedo)
uniform sampler2D us_normal;
uniform sampler2D us_displacement;

uniform vec<3> u_light_dir;
uniform vec<3> u_tint;

void main()
{
    vec<3> rgb = texture(us_color, te_texcoord).rgb * u_tint;
    vec<3> tn = normalize((texture(us_normal, te_texcoord).xyz * 2.0) - 1.0);
    mat3 tbn = mat3(te_binormal, te_tangent, te_normal);

    vec<3> normal = tbn * tn;
    //vec<3> normal = tbn * vec<3>(0.0, 0.0, 1.0);

    float shade = (dot(normalize(-u_light_dir), normal) + 1.0) / 2.0;
    float l = shade * 0.75 +  0.25;
	// color = vec4(l, 0.0, 0.0, 1.0);
    // color = vec4(vec<3>(v_texcoord, 1.0), 1.0);
    color = vec4(rgb.xyz * l, 1.0);
    // color = vec4(l * ((normal.xyz / 2.0) + 0.5), 1.0);
    // color = vec4(((normal.xyz / 2.0) + 0.5), 1.0);
}
