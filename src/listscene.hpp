#pragma once

#include "core.h"

namespace seen
{

class ListScene : public Scene
{
public:
	ListScene() = default;
    ListScene(std::initializer_list<const Drawable*> drawables);
	~ListScene() = default;

    void insert(const Drawable* d) override;
    void erase(const Drawable* d) override;

    std::vector<const Drawable*>& all() override;

	void draw() const override;

private:
    std::vector<const Drawable*> _drawables;
};

}
