#pragma once

#include "core.h"
#include "listscene.hpp"
#include "cubemap.hpp"
#include "light.hpp"

namespace seen
{

class CustomPass : public RenderingPass
{
public:
	CustomPass();
	CustomPass(std::function<void (int)> prep);
	~CustomPass();

	void prepare(int index);
	void draw(Viewer* viewer);
	void finish() {}

	int instances;

	std::function<void (int)> preparation_function;
};

class ShadowPass : public RenderingPass
{
	friend struct ShaderProgram;

public:
	ShadowPass(int resolution=1024, bool generate_mipmaps=true);
	~ShadowPass();

	void prepare(int index);
	void draw(Viewer* viewer);
	void finish();

	// Scene* scene;
	std::vector<Light*> lights;

private:
	Cubemap* _cubemap;
	bool _generate_mipmaps;
};

}
