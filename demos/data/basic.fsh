#version 330 core

in vec2 v_texcoord; // texture coords
in vec3 v_normal;   // normal
in vec3 v_tangent;  // tangent
in vec3 v_binormal; // binormal (for TBN basis calc)
in vec3 v_pos;      // pixel view space position

out vec4 color;

uniform sampler2D tex;     // base texture (albedo)
uniform sampler2D norm;

uniform vec3 light_dir;
uniform float u_green;

void main()
{
    vec3 rgb = texture(tex, v_texcoord).xyz * vec3(u_green, u_green, u_green);
    vec3 tn = normalize((texture(norm, v_texcoord).xyz * 2.0) - 1.0);
    mat3 tbn = mat3(v_tangent, v_binormal, v_normal);

    vec3 normal = tbn * tn;
    //vec3 normal = tbn * vec3(0.0, 0.0, 1.0);

    float shade = (dot(normalize(-light_dir), normal) + 1.0) / 2.0;
    float l = shade;// * 0.9 + 0.1;
    // color = vec4(vec3(v_texcoord, 1.0), 1.0);
    color = vec4(rgb * shade, 1.0);
    //color = vec4(l * ((normal.xyz / 2.0) + 0.5), 1.0);
}
