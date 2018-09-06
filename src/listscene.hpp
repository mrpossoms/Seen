#pragma once

#include "core.h"

namespace seen
{

class ListScene : public Scene
{
public:
	ListScene() = default;
    ListScene(std::initializer_list<Drawable*> drawables);
	~ListScene() = default;

    void insert(Drawable* d);
    void erase(Drawable* d);

    std::vector<Drawable*>& all();

	void draw();

private:
    std::vector<Drawable*> _drawables;
};

}
