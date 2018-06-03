#include "sky.hpp"

using namespace seen;

Sky::Sky()
{

}
//------------------------------------------------------------------------------

Sky::~Sky()
{
    glDeleteBuffers(1, &vbo);
}
//------------------------------------------------------------------------------

void Sky::draw(Viewer* viewer)
{
	assert(gl_get_error());

    Model* model = MeshFactory::get_model("sphereized_cube.obj");

	mat4x4_t world;
	mat3x3_t rot;
	mat4x4_identity(world.v);
	mat3x3_identity(rot.v);

    ShaderProgram& prog = *ShaderProgram::active();

    prog["u_normal_matrix"] << rot;
    prog["u_world_matrix"] << world;

    glDisable(GL_CULL_FACE);
    model->draw(viewer);
    glEnable(GL_CULL_FACE);
    assert(gl_get_error());
}
