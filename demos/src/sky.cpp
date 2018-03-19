#include "sky.hpp"

using namespace seen;

Sky::Sky()
{
    glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

    Mesh* mesh = MeshFactory::get_model("data/sphereized_cube.obj");
    vertices = mesh->vert_count();

	glBufferData(
		GL_ARRAY_BUFFER,
		mesh->vert_count() * sizeof(Vertex),
		mesh->verts(),
		GL_STATIC_DRAW
	);
}
//------------------------------------------------------------------------------

Sky::~Sky()
{
    glDeleteBuffers(1, &vbo);
}
//------------------------------------------------------------------------------

void Sky::draw(Viewer* viewer)
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
    glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	for(int i = 4; i--;)
	{
		glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3) * i));
	}


	assert(gl_get_error());

	mat4x4 world;
	mat3x3 rot;
	mat4x4_identity(world);
	mat3x3_identity(rot);

    DrawParams& params = ShaderProgram::active()->draw_params;

	glUniformMatrix4fv(params.world_uniform, 1, GL_FALSE, (GLfloat*)world);
	glUniformMatrix3fv(params.norm_uniform,  1, GL_FALSE, (GLfloat*)rot);

	glDrawArrays(GL_TRIANGLES, 0, vertices);

	assert(gl_get_error());

	for(int i = 4; i--;)
	{
		glDisableVertexAttribArray(i);
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	assert(gl_get_error());
}
