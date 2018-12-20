#include "listscene.hpp"

using namespace seen;


ListScene::ListScene(std::initializer_list<const Drawable*> drawables)
{
    for (auto drawable : drawables)
    {
        _drawables.push_back(drawable);
    }
}


void ListScene::insert(const Drawable* d)
{
    _drawables.push_back(d);
}


void ListScene::erase(const Drawable* d)
{
    auto it = find(_drawables.cbegin(), _drawables.cend(), d);
    _drawables.erase(it);
}


std::vector<const seen::Drawable*>& ListScene::all()
{
    return _drawables;
}


void ListScene::draw() const
{
	for (auto drawable : _drawables)
	{
		drawable->draw();
	}
}
