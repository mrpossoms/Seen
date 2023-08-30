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

	mat<4,4> world;
	mat<3,3> rot;
	mat<4, 4>_identity(world.v);
	mat<3, 3>_identity(rot.v);

    ShaderProgram& prog = *ShaderProgram::active();

    prog["u_normal_matrix"] << rot;
    prog["u_world_matrix"] << world;

    glDisable(GL_CULL_FACE);
    model->draw(viewer);
    glEnable(GL_CULL_FACE);
    assert(gl_get_error());
}
