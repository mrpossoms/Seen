#include "geo.hpp"
#include "shader.hpp"

using namespace seen;

//------------------------------------------------------------------------------
unsigned int Mesh::vert_count()
{
	return vertices.size();
}

//------------------------------------------------------------------------------
unsigned int Mesh::index_count()
{
	return indices.size();
}

//------------------------------------------------------------------------------
Vertex* Mesh::verts()
{
	return vertices.data();
}

//------------------------------------------------------------------------------
uint16_t* Mesh::inds()
{
	return indices.data();
}

//------------------------------------------------------------------------------
void Mesh::compute_normals()
{
	Vertex* v = verts();

	for(unsigned int i = 0; i < indices.size(); i += 3)
	{
		vec3 diff[2];

		vec3_sub(diff[0], v[indices[i + 0]].position, v[indices[i + 1]].position);
		vec3_sub(diff[1], v[indices[i + 0]].position, v[indices[i + 2]].position);

		Vertex* vert = v + indices[i];
		vec3 cross;
		vec3_mul_cross(cross, diff[0], diff[1]);
		vec3_norm(vert->normal, cross);

		if(isnan(vert->normal[0]) || isnan(vert->normal[1]) || isnan(vert->normal[2]))
		{
			// assert(0);
		}

		i = (i + 1) - 1;
	}
}
//------------------------------------------------------------------------------

void Mesh::compute_tangents()
{
	Vertex* v = verts();

	for(unsigned int i = 0; i < indices.size(); i += 3)
	{

		vec3_sub(v[indices[i + 0]].tangent, v[indices[i]].position, v[indices[i + 1]].position);

		for(int j = 3; j--;)
		{
			vec3_norm(v[indices[i + j]].tangent, v[indices[i + j]].tangent);
		}
	}
}
//------------------------------------------------------------------------------

Vec3 Mesh::min_position()
{
	if(_min) return *_min;
	Vertex* v = verts();
	_min = new Vec3(v->position[0], v->position[1], v->position[2]);

	for(int i = vert_count(); i--;)
	{
		for(int j = 3; j--;)
		{
			if(v[i].position[j] < _min->v[j])
			{
				_min->v[j] = v[i].position[j];
			}
		}
	}

	return *_min;
}

//------------------------------------------------------------------------------
Vec3 Mesh::max_position()
{
	if(_max) return *_max;
	Vertex* v = verts();
	_max = new Vec3(v->position[0], v->position[1], v->position[2]);

	for(int i = vert_count(); i--;)
	{
		for(int j = 3; j--;)
		{
			if(v[i].position[j] > _max->v[j])
			{
				_max->v[j] = v[i].position[j];
			}
		}
	}

	return *_max;
}


//------------------------------------------------------------------------------
Vec3 Mesh::box_dimensions()
{
	Vec3 min = min_position();
	Vec3 max = max_position();
	return max - min;
}


//------------------------------------------------------------------------------
//    ___ _____ _
//   / __|_   _| |
//   \__ \ | | | |__
//   |___/ |_| |____|
//
STLMesh::STLMesh(int fd)
{
	assert(fd > 0);

	// read the header
	assert(read(fd, this->header, STL_HEADER_SIZE) == STL_HEADER_SIZE);

	// get the number of triangles
	assert(read(fd, &this->tri_count, sizeof(this->tri_count)) == sizeof(this->tri_count));

	// allocate space for the model's geometry
	unsigned int vert_count = this->tri_count * 3;
	all_verts = new Vertex[vert_count];
	all_positions = new Vec3[vert_count];
	all_normals = new Vec3[this->tri_count];
	this->tris = new STLTri[this->tri_count];
	this->_indices = new unsigned int[vert_count];

	// fill out a list of indicies, one for each vert
	// doing this just to appease ode. Since stl files
	// have no such concept
	for(int i = vert_count; i--;)
	{
		_indices[i] = i;
	}

	assert(this->tris && all_positions && all_normals);

	// read all the geometry
	for(unsigned int i = 0; i < this->tri_count; ++i)
	{
		STLTri* tri = this->tris + i;

		// point the verts and normal at the appropriate
		// spot allocated in the contiguious arrays
		tri->verts = all_positions + (i * 3);
		tri->normal = all_normals + i;

		// read from file
		read(fd, tri->normal, sizeof(Vec3));
		read(fd, tri->verts, sizeof(Vec3) * 3);
		read(fd, &tri->attr, sizeof(tri->attr));

		// Copy positions and normals into contiguious array
		// for rendering
		vec3_copy(all_verts[(i * 3) + 0].position , tri->verts[0].v);
		vec3_copy(all_verts[(i * 3) + 0].normal   , tri->normal->v);
		vec3_copy(all_verts[(i * 3) + 1].position , tri->verts[1].v);
		vec3_copy(all_verts[(i * 3) + 1].normal   , tri->normal->v);
		vec3_copy(all_verts[(i * 3) + 2].position , tri->verts[2].v);
		vec3_copy(all_verts[(i * 3) + 2].normal   , tri->normal->v);
	}

	compute_tangents();
}
//------------------------------------------------------------------------------
STLMesh::~STLMesh()
{
	delete this->tris[0].verts;
	delete this->tris[0].normal;
	delete this->tris;
	delete this->_indices;
}

//------------------------------------------------------------------------------
unsigned int STLMesh::vert_count()
{
	return tri_count * 3;
}

//------------------------------------------------------------------------------
Vertex* STLMesh::verts()
{
	return NULL;
}

//------------------------------------------------------------------------------
Plane::Plane(float size)
{
	Vertex verts[] = {
		{ { -1 * size, 1 * size,  0 } },
		{ {  1 * size, 1 * size,  0 } },
		{ { -1 * size,-1 * size,  0 } },

		{ { -1 * size,-1 * size,  0 } },
		{ {  1 * size, 1 * size,  0 } },
		{ {  1 * size,-1 * size,  0 } },
	};

	for(int i = 6; i--;)
	{
		verts[i].normal[2] = 1;
		verts[i].texture[0] = 10 * ((verts[i].position[0] / size) + 1) / 2;
		verts[i].texture[1] = 10 * ((verts[i].position[1] / size) + 1) / 2;
		vertices.push_back(verts[i]);
	}

	// compute_tangents();
}

//------------------------------------------------------------------------------
Plane::Plane(float size, int subdivisions)
{
	float dx = 1 / (float)subdivisions;
	float dy = 1 / (float)subdivisions;

	for (int i = 0; i < subdivisions; i++)
	for (int j = 0; j < subdivisions; j++)
	{
		float x = dx * i - 0.5f, y = dy * j - 0.5f;
		Vertex v = {
			.position = { x * size, 0, y * size },
			.normal = { 0, 1, 0 },
			.tangent = { 1, 0, 0 },
			.texture = { dx * i * size, dy * j * size, 0 }
		};

		vertices.push_back(v);
	}

	for (int y = 0; y < subdivisions - 1; y++)
	for (int x = 0; x < subdivisions - 1; x++)
	{
		int i = x + y * (subdivisions);
		int j = x + (y + 1) * (subdivisions);
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(j);

		indices.push_back(j + 1);
		indices.push_back(j);
		indices.push_back(i + 1);
	}
}
//------------------------------------------------------------------------------

Plane::Plane(Vec3 c0, Vec3 c1)
{
	Vertex verts[4] = {
		{ { c0.x, c0.y, c0.z } },
		{ { c1.x, c0.y, c1.z } },
		{ { c0.x, c1.y, c0.z } },
		{ { c1.x, c1.y, c1.z } },
	};

	vec3_t normal, d[2];
	vec3_sub(d[0].v, verts[0].position, verts[1].position);
	vec3_sub(d[1].v, verts[0].position, verts[2].position);
	vec3_mul_cross(normal.v, d[0].v, d[1].v);
	vec3_norm(verts[0].normal, normal.v);

	for(int i = 4; i--;)
	{
		if (i > 0)
		{
			vec3_copy(verts[i].normal, verts[0].normal);
		}

		printf("%f %f %f\n", verts[i].normal[0], verts[i].normal[1], verts[i].normal[2]);

		verts[i].texture[0] = 10 * ((verts[i].position[0]) + 1) / 2;
		verts[i].texture[1] = 10 * ((verts[i].position[1]) + 1) / 2;
		vertices.push_back(verts[i]);
	}

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);

	// compute_tangents();
}

//------------------------------------------------------------------------------
Heightmap::Heightmap(std::string path, float size, int resolution) :
           Plane(size, resolution)
{
	auto texture = TextureFactory::load_texture(path);
	generate(texture, resolution);
}

//------------------------------------------------------------------------------
Heightmap::Heightmap(Tex texture, float size, int resolution) :
           Plane(size, resolution)
{
	generate(texture, resolution);
}

//------------------------------------------------------------------------------
void Heightmap::generate(Tex texture, int resolution)
{
	struct rgba_t {
		uint8_t r, g, b, a;
	};

	struct r_t {
		uint8_t r;
	};

	rgba_t textels[resolution * resolution];
	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexImage(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		(void*)textels
	);

	for (int y = 0; y < resolution; y++)
	for (int x = 0; x < resolution; x++)
	{
		int ti = x + y * (resolution);
		int vi = x + y * (resolution);

		verts()[vi].position[1] = textels[ti].r / 255.f;
	}

	compute_normals();
}

//------------------------------------------------------------------------------
Volume::Volume(Vec3 corner0, Vec3 corner1, int divisions)
{
	_corners[0] = corner0;
	_corners[1] = corner1;
	_divisions = divisions + 1;

	// reserve space for the maximum number of tris that could be allocated
	// int max_possible_tris = pow(8, divisions) * 5;
	// int max_possible_verts = max_possible_tris * 3;
	// vertices.reserve(max_possible_verts);
}

//------------------------------------------------------------------------------
void Volume::generate(float(*density_at)(vec3 loc))
{
	#include "mc_luts.hpp"

	float div = _divisions;

	Vec3& p0 = _corners[0];
	Vec3& p1 = _corners[1];
	Vec3 block_delta = (p1 - p0);
	Vec3 voxel_delta = block_delta / div;

	for (int x = _divisions; x--;)
	for (int y = _divisions; y--;)
	for (int z = _divisions; z--;)
	{
		Vec3 voxel_index(x, y, z);
		Vec3 p[8];  // voxel corners
		float d[8]; // densities at each corner
		uint8_t voxel_case = 0;

		Vec3 c[8] = {
			{ 0.f, 0.f, 0.f },
			{ 0.f, 1.f, 0.f },
			{ 1.f, 1.f, 0.f },
			{ 1.f, 0.f, 0.f },

			{ 0.f, 0.f, 1.f },
			{ 0.f, 1.f, 1.f },
			{ 1.f, 1.f, 1.f },
			{ 1.f, 0.f, 1.f },
		};

		// compute positions of the corners for voxel x,y,z as well
		// as the case for the voxel
		for (int i = 8; i--;)
		{
			c[i] *= voxel_delta;
			p[i] = p0 + (voxel_delta * voxel_index) + c[i];
			d[i] = density_at(p[i].v);

			if (d[i] <= 0)
			{
				voxel_case |= (1 << i);
			}
		}

		// compute lerp weights between verts for each edge
		float w[12];
		for (int i = 0; i < 12; ++i)
		{
			int e_i = tri_edge_list_case[voxel_case][i];

			if (e_i == -1) break;

			// v0 * w + v1 * (1 - w)
			// w = 0.5
			// -1 * w + 1 * (1 - w) = 0
			//
			// d0 * w + d1 * (1 - w) = 0
			// d0 * w + d1 - d1 * w = 0
			// (d0 * w - d1 * w) / w = -d1 / w
			// d0 - d1 = -d1 / w
			// -d1 / (d0 - d1) = w

			int p0_i = edge_list[e_i][0];
			int p1_i = edge_list[e_i][1];

			// solve for the weight that will lerp between
			w[i] = d[p0_i] / (d[p0_i] - d[p1_i]);

			Vertex v = {};
			Vec3 _p = p[p1_i] * w[i] + p[p0_i] * (1 - w[i]);
			vec3_copy(v.position, _p.v);

			indices.push_back(vertices.size());
			vertices.push_back(v);
		}

	}

	// use the gradient of the density function to compute the
	// normal vector for each vertex
	for (int i = 0; i < vertices.size(); i++)
	{
		const float s = 0.1;
		vec3 grad;
		vec3 deltas[3][2] = {
			{{ s, 0, 0 }, { -s, 0, 0 }},
			{{ 0, s, 0 }, { 0, -s, 0 }},
			{{ 0, 0, s }, { 0,  0, -s }},
		};
		Vertex& v = vertices[i];

		for (int j = 3; j--;)
		{
			vec3 samples[2];
			vec3_add(samples[0], v.position, deltas[j][0]);
			vec3_add(samples[1], v.position, deltas[j][1]);
			grad[j] = density_at(samples[0]) - density_at(samples[1]);
		}

		vec3_norm(v.normal, grad);
	}
}
//------------------------------------------------------------------------------
//     ___  ___    _
//    / _ \| _ )_ | |
//   | (_) | _ \ || |
//    \___/|___/\__/
//
enum ObjLineType {
	COMMENT = 0,
	POSITION,
	TEXTURE,
	NORMAL,
	PARAMETER,
	FACE,
	UNKNOWN,
};


struct ObjLine {
	ObjLineType type;
	char str[1024];
	union {
		vec3 position;
		vec3 texture;
		vec3 normal;
		vec3 parameter;
		struct {
			int pos_idx[3], tex_idx[3], norm_idx[3];
		} face;
	};
};

//------------------------------------------------------------------------------
static int get_line(int fd, char* line)
{
	int size = 0;
	while(read(fd, line + size, 1))
	{
		if(line[size] == '\n') break;
		++size;
	}

	line[size] = '\0';

	return size;
}

//------------------------------------------------------------------------------
bool parse_line(int fd, ObjLine& line)
{
	char *save_ptr = NULL;
	line.type = UNKNOWN;

	if(get_line(fd, line.str) == 0)
	{
		return false;
	}

	char* token = strtok_r(line.str, " ", &save_ptr);
	if(!token) return false;

	// Determine the tag of the line
	const char* tag[] = { R"(#)", R"(v)", R"(vt)", R"(vn)", R"(vp)", R"(f)", NULL };
	for(int i = 0; tag[i]; ++i)
	{
		if(strcmp(tag[i], token) == 0)
		{
			line.type = (ObjLineType)i;
			break;
		}
	}

	float* v;
	int vec_size = 0;
	switch (line.type)
	{
		case COMMENT:
			break;
		case POSITION:
			v = line.position;
			vec_size = 3;
			break;
		case TEXTURE:
			v = line.texture;
			vec_size = 2;
			break;
		case NORMAL:
			v = line.normal;
			vec_size = 3;
			break;
		case PARAMETER:
			v = line.parameter;
			vec_size = 3;
			break;
		case FACE:
		bzero(&line.face, sizeof(line.face));
		for(int i = 0; i < 3; ++i)
		{
			token = strtok_r(NULL, " ", &save_ptr);
			if(!token) break;

			char* idx_token = token;
			for(int j = strlen(idx_token); j--;)
			{
				if(idx_token[j] == '/') idx_token[j] = '\0';
			}

			for(int j = 0; j < 3; ++j)
			{
				if(*idx_token != '\0') switch (j)
				{
					case 0:
						sscanf(idx_token, "%d", &line.face.pos_idx[i]);
						break;
					case 1:
						sscanf(idx_token, "%d", &line.face.tex_idx[i]);
						break;
					case 2:
						sscanf(idx_token, "%d", &line.face.norm_idx[i]);
						break;
				}

				idx_token += strlen(idx_token) + 1;
			}

			// printf("%d/%d/%d\n", line.face.pos_idx[i], line.face.tex_idx[i], line.face.norm_idx[i]);
		}
			break;
		default:;
	}

	// Read the vector selected above
	for(int i = 0; i < vec_size; ++i)
	{
		token = strtok_r(NULL, " ", &save_ptr);
		// printf("type: %d TOK: '%s' %d\n", line.type, token, vec_size);
		sscanf(token, "%f", v + i);
	}

	return true;
}

//------------------------------------------------------------------------------
OBJMesh::OBJMesh(int fd)
{
	_min = _max = NULL;

	std::map<std::string, uint16_t> index_map;

	ObjLine l = {};
	while(parse_line(fd, l))
	{
		switch (l.type)
		{
			case COMMENT:
				// printf("%s\n", l.str);
				break;
			case POSITION:
			{
				vec3_t p = { { l.position[0], l.position[1], l.position[2] } };
				positions.push_back(p);
			}
				break;
			case TEXTURE:
			{
				vec3_t t = { { l.texture[0], l.texture[1], l.texture[2] } };
				tex_coords.push_back(t);
			}
				break;
			case NORMAL:
			{
				// printf("n %f %f %f\n", l.normal[0], l.normal[1], l.normal[2] } };
				vec3_t n = { { l.normal[0], l.normal[1], l.normal[2] } };
				normals.push_back(n);
			}
				break;
			case PARAMETER:
			{
				vec3_t p = { { l.parameter[0], l.parameter[1], l.parameter[2] } };
				params.push_back(p);
			}
				break;
			case FACE:
			{
				for(int i = 0; i < 3; ++i)
				{
					Vertex v = {};
					if(l.face.pos_idx[i])  vec3_copy(v.position, positions[l.face.pos_idx[i] - 1].v);
					if(l.face.tex_idx[i])  vec3_copy(v.texture,  tex_coords[l.face.tex_idx[i] - 1].v);
					if(l.face.norm_idx[i]) vec3_copy(v.normal,   normals[l.face.norm_idx[i] - 1].v);

					char token[21] = {};

					snprintf(token, sizeof(token), "%u/%u/%u",
					         l.face.pos_idx[i],
							 l.face.tex_idx[i],
							 l.face.norm_idx[i]
					);

					std::string vert_token(token);

					if (index_map.count(vert_token) == 0)
					{
						vertices.push_back(v);
						index_map[vert_token] = (uint16_t)(vertices.size() - 1);
					}

					indices.push_back(index_map[vert_token]);
				}
			}
				break;
			case UNKNOWN:
				break;
		}
	}

	compute_tangents();
}

//------------------------------------------------------------------------------
OBJMesh::~OBJMesh()
{

}

//------------------------------------------------------------------------------
//    ___        _
//   | __|_ _ __| |_ ___ _ _ _  _
//   | _/ _` / _|  _/ _ \ '_| || |
//   |_|\__,_\__|\__\___/_|  \_, |
//                           |__/
Mesh* MeshFactory::get_mesh(std::string path)
{
	static std::map<std::string, Mesh*> _cached_models;

	// try to open this file
	std::string full_path = DATA_PATH + "/" + path;
	int fd = open(full_path.c_str(), O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "Failed to open '%s'\n", full_path.c_str());
		return NULL;
	}

	// Find this path's file ext
	const char* ext = NULL;
	for(int i = full_path.length() - 1; full_path.at(i); i--) if(full_path.at(i) == '.')
	{
		ext = full_path.c_str() + i + 1;
		break;
	}

	if(ext == NULL)
	{
		fprintf(stderr, "Failed find to path's extension '%s'\n", full_path.c_str());
		return NULL;
	}

	int matched_ext = -1;
	const std::string exts[] = { "stl", "obj" };
	for(int i = 2; i--;)
	{
		if(strcmp(exts[i].c_str(), ext) == 0)
		{
			matched_ext = i;
			break;
		}
	}

	if(_cached_models.count(path) == 0)
	{
		switch (matched_ext)
		{
			case 0:
				_cached_models[path] = new STLMesh(fd);
				break;
			case 1:
				_cached_models[path] = new OBJMesh(fd);
				break;
			default:
				fprintf(stderr, "No loader matched\n");
		}
	}

	assert(_cached_models[path]);

	return _cached_models[path];
}
//------------------------------------------------------------------------------

Model* MeshFactory::get_model(std::string path)
{
	static std::map<std::string, Model*> _cached_models;

	if(_cached_models.count(path) == 0)
	{
		_cached_models[path] = new Model(MeshFactory::get_mesh(path));
	}

	assert(_cached_models[path]);

	return _cached_models[path];
}


Model::Model(Mesh* mesh)
{
	glGenBuffers(2, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	assert(mesh);
	glBufferData(
		GL_ARRAY_BUFFER,
		mesh->vert_count() * sizeof(Vertex),
		mesh->verts(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		mesh->index_count() * sizeof(uint16_t),
		mesh->inds(),
		GL_STATIC_DRAW
	);

	vertices = mesh->vert_count();
	indices  = mesh->index_count();
}
//------------------------------------------------------------------------------

Model::~Model()
{
    glDeleteBuffers(1, &vbo);
}
//------------------------------------------------------------------------------

void Model::draw()
{
	assert(gl_get_error());

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	assert(gl_get_error());

	glEnableVertexAttribArray(0);
	assert(gl_get_error());
	glEnableVertexAttribArray(1);
	assert(gl_get_error());
	glEnableVertexAttribArray(2);
	assert(gl_get_error());
	glEnableVertexAttribArray(3);
	assert(gl_get_error());

	for(int i = 4; i--;)
	{
		glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3) * i));
	}


	assert(gl_get_error());

	*ShaderProgram::active() << (Positionable*)this;

	assert(gl_get_error());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glPatchParameteri(GL_PATCH_VERTICES, 3);
	assert(gl_get_error());
	// glDrawArrays(GL_PATCHES, 0, vertices);
	glDrawElements(ShaderProgram::active()->primative, indices, GL_UNSIGNED_SHORT, 0);

	assert(gl_get_error());

	for(int i = 4; i--;)
	{
		glDisableVertexAttribArray(i);
	}

	assert(gl_get_error());
}
