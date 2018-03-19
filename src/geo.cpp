#include "geo.hpp"

using namespace seen;

//------------------------------------------------------------------------------
unsigned int Mesh::vert_count()
{
	return vertices.size();
}

//------------------------------------------------------------------------------
Vertex* Mesh::verts()
{
	return vertices.data();
}

//------------------------------------------------------------------------------
void Mesh::compute_tangents()
{
	Vertex* v = verts();

	for(int i = 0; i < vert_count(); i += 3)
	{
		vec3_sub(v[i + 0].tangent, v[i].position, v[i + 1].position);
		vec3_sub(v[i + 1].tangent, v[i].position, v[i + 1].position);
		vec3_sub(v[i + 2].tangent, v[i].position, v[i + 1].position);

		for(int j = 3; j--;)
		{
			// vec3_mul_cross(v[i + j].tangent, v[i + j].tangent, v[i + j].normal);
			vec3_norm(v[i + j].tangent, v[i + j].tangent);
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
	for(int i = 0; i < this->tri_count; ++i)
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
Plane::~Plane()
{

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
	char *save_ptr = NULL, *sub_save_ptr = NULL;
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
				vec3_t p = { l.position[0], l.position[1], l.position[2] };
				positions.push_back(p);
			}
				break;
			case TEXTURE:
			{
				vec3_t t = { l.texture[0], l.texture[1], l.texture[2] };
				tex_coords.push_back(t);
			}
				break;
			case NORMAL:
			{
				// printf("n %f %f %f\n", l.normal[0], l.normal[1], l.normal[2] };
				vec3_t n = { l.normal[0], l.normal[1], l.normal[2] };
				normals.push_back(n);
			}
				break;
			case PARAMETER:
			{
				vec3_t p = { l.parameter[0], l.parameter[1], l.parameter[2] };
				params.push_back(p);
			}
				break;
			case FACE:
			{
				// printf("FACE\n");


				for(int i = 0; i < 3; ++i)
				{
					Vertex v = {};
					if(l.face.pos_idx[i])  vec3_copy(v.position, positions[l.face.pos_idx[i] - 1].v);
					if(l.face.tex_idx[i])  vec3_copy(v.texture,  tex_coords[l.face.tex_idx[i] - 1].v);
					if(l.face.norm_idx[i]) vec3_copy(v.normal,   normals[l.face.norm_idx[i] - 1].v);

					vertices.push_back(v);
					indices.push_back(l.face.pos_idx[i]);
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
Mesh* MeshFactory::get_model(std::string path)
{
	static std::map<std::string, Mesh*> _cached_models;

	// try to open this file
	int fd = open(path.c_str(), O_RDONLY);
	if(fd < 0)
	{
		fprintf(stderr, "Failed to open '%s'\n", path.c_str());
		return NULL;
	}

	// Find this path's file ext
	const char* ext = NULL;
	for(int i = path.length() - 1; path.at(i); i--) if(path.at(i) == '.')
	{
		ext = path.c_str() + i + 1;
		break;
	}
	if(ext == NULL) return NULL;

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
		}
	}

	return _cached_models[path];
}
