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
	vec<3>* normal;
	vec<3>* verts;
	uint16_t attr;
};
//------------------------------------------------------------------------------
struct STLVert
{
	vec<3> position;
	vec<3> normal;
};
//------------------------------------------------------------------------------
struct Vertex
{
	vec<3> position;
	vec<3> normal;
	vec<3> tangent;
	vec<3> texture;
};
//------------------------------------------------------------------------------
struct Mesh
{
	unsigned int vert_count();
	unsigned int index_count();
	Vertex* verts();
	uint16_t* inds();

	vec<3> *_min, *_max;

	void compute_normals();
	void compute_tangents();

	vec<3> min_position();
	vec<3> max_position();
	vec<3> box_dimensions();

protected:
	std::vector<vec<3>> positions;
	std::vector<vec<3>> tex_coords;
	std::vector<vec<3>> normals;

	std::vector<vec<3>> params;
	std::vector<uint16_t> indices;
	std::vector<Vertex> vertices;
};

//------------------------------------------------------------------------------
struct Model : Drawable, Positionable
{
public:
	Model(Mesh* mesh);
	~Model();

	void draw();
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
	vec<3>* all_positions;
	vec<3>* all_normals;
	STLTri* tris;

	STLMesh(int fd);
	~STLMesh();
	unsigned int vert_count();
	Vertex* verts();

	vec<3> min_position();
	vec<3> max_position();

private:
	vec<3> *_min, *_max;
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
	Plane(vec<3> corner_0, vec<3> corner_1);
	~Plane() = default;
};

//------------------------------------------------------------------------------
struct Heightmap : Plane
{
	Heightmap(std::string path, float size, int resolution);
	Heightmap(Tex texture, float size, int resolution);
	~Heightmap();

private:
	void generate(Tex t, int resolution);
};

//------------------------------------------------------------------------------
struct Volume : Mesh
{
	Volume(vec<3> corner0, vec<3> corner1, int divisions);

	void generate(float(*density_at)(vec<3> loc));
private:
	int _divisions;
	vec<3> _corners[2];
};

}
