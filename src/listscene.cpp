#include "listscene.hpp"

using namespace seen;


ListScene::ListScene(std::initializer_list<Drawable*> drawables)
{
    for (Drawable* drawable : drawables)
    {
        _drawables.push_back(drawable);
    }
}


void ListScene::insert(Drawable* d)
{
    _drawables.push_back(d);
}


void ListScene::erase(Drawable* d)
{
    auto it = find(_drawables.begin(), _drawables.end(), d);
    _drawables.erase(it);
}


std::vector<seen::Drawable*>& ListScene::all()
{
    return _drawables;
}


void ListScene::draw()
{
	for (Drawable* drawable : _drawables)
	{
		drawable->draw();
	}
}
