#pragma once

#include "core.h"

namespace seen
{

class ListScene : public Scene
{
public:
	ListScene() = default;
	~ListScene() = default;

	std::vector<Drawable*>& drawables();
	void draw();

private:
	std::vector<Drawable*> _drawables;
};

}
