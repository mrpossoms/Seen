#include "listscene.hpp"

using namespace seen;

std::vector<seen::Drawable*>& seen::ListScene::drawables()
{
	return _drawables;
}


void ListScene::draw(Viewer* viewer)
{
	for (Drawable* drawable : _drawables)
	{
		drawable->draw(viewer);
	}
}
