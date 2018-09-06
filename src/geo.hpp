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

	void compute_normals();
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
	Plane(Vec3 corner_0, Vec3 corner_1);
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
	Volume(Vec3 corner0, Vec3 corner1, int divisions);

	void generate(float(*density_at)(vec3 loc));
private:
	int _divisions;
	Vec3 _corners[2];
};

}
