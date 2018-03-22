#pragma once

#include "core.h"
#include "listscene.hpp"

namespace seen
{

class CustomPass : public RenderingPass
{
public:
	CustomPass();
	~CustomPass();

	void prepare();
	void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding) {}
	void draw(Viewer* viewer);

	std::function<void (void)>* preparation_function;
	std::vector<Drawable*>* drawables;
};

}
