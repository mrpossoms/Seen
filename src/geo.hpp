#pragma once

#include "core.h"
#include "texture.hpp"

#define STL_HEADER_SIZE 80

namespace seen
{

//    ___ _____ _
//   / __|_   _| |
//   \__ \ | | | |__
//   |___/ |_| |____|
//
struct STLTri
{
	Vec3* normal;
	Vec3* verts;
	uint16_t attr;
};
//------------------------------------------------------------------------------
struct STLVert
{
	Vec3 position;
	Vec3 normal;
};
//------------------------------------------------------------------------------
struct Vertex
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 texture;
};
//------------------------------------------------------------------------------
struct Mesh
{
	unsigned int vert_count();
	unsigned int index_count();
	Vertex* verts();
	uint16_t* inds();

	Vec3 *_min, *_max;

	void compute_tangents();

	Vec3 min_position();
	Vec3 max_position();
	Vec3 box_dimensions();

protected:
	std::vector<vec3_t> positions;
	std::vector<vec3_t> tex_coords;
	std::vector<vec3_t> normals;

	std::vector<vec3_t> params;
	std::vector<uint16_t> indices;
	std::vector<Vertex> vertices;
};

//------------------------------------------------------------------------------

struct Model : Drawable
{
public:
	Model(Mesh* mesh);
	~Model();

	void draw(Viewer* viewer);
private:
	GLuint vbo, ibo;
	unsigned int vertices, indices;
};

//------------------------------------------------------------------------------
struct STLMesh : Mesh
{
	uint8_t header[STL_HEADER_SIZE];
	uint32_t tri_count;

	Vertex* all_verts;
	Vec3* all_positions;
	Vec3* all_normals;
	STLTri* tris;

	STLMesh(int fd);
	~STLMesh();
	unsigned int vert_count();
	Vertex* verts();

	Vec3 min_position();
	Vec3 max_position();

private:
	Vec3 *_min, *_max;
	unsigned int* _indices;
};

//------------------------------------------------------------------------------
class MeshFactory
{
public:
	static Mesh* get_mesh(std::string path);
	static Model* get_model(std::string path);
};

//------------------------------------------------------------------------------
struct OBJMesh : Mesh
{
	OBJMesh(int fd);
	~OBJMesh();

	unsigned int vert_count();
	Vertex* verts();
};

//------------------------------------------------------------------------------
struct Plane : Mesh
{
	Plane(float size);
	Plane(float size, int subdivisions);
	~Plane() = default;
};

//------------------------------------------------------------------------------
struct Heightmap : Plane
{
	Heightmap(Tex texture, float size, int resolution);
	~Heightmap();
};

}
