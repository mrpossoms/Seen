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
	CustomPass(std::function<void (int)> prep, ...);
	~CustomPass();

	void prepare(int index);
	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding) {}
	void draw(Viewer* viewer);

	int instances;

	std::function<void (int)> preparation_function;
	std::vector<Drawable*> drawables;
};

class ShadowPass : public RenderingPass
{
public:
	ShadowPass(int resolution=1024);
	~ShadowPass();

	void prepare(int index);
	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding) {}
	void draw(Viewer* viewer);

	Scene* scene;
	std::vector<Light*> lights;

private:
	Cubemap* _cubemap;
};

}
