#pragma once

#include <GLFW/glfw3.h>

#define XMTYPE float
#include "xmath.h"

using namespace xmath;


namespace seen
{

struct Positionable
{
		// Dynamic Interface
		vec<3>& position();
		vec<3> left();
		vec<3> forward();
		quat<> orientation();

		Positionable* position(vec<3>& pos);
		Positionable* position(float x, float y, float z);

		Positionable* orientation(quat<>& ori);
		void orientation(mat<3, 3> rot);

		void world(mat<4, 4> world);
		mat<4,4>& world();

		mat<3,3> normal_matrix = { {
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 },
		} };
private:
		vec<3> _position;
		quat<> _orientation;
		mat<4,4> _world = { {
			{ 1, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 0, 1 }
		} };
};


class Viewer : public Positionable
{
public:
	virtual Viewer* view_projection(mat<4, 4> vp) = 0;
	virtual Viewer* view(mat<4, 4> v) = 0;
	virtual Viewer* projection(mat<4, 4> p) = 0;

	mat<4,4> _view;
	mat<4,4> _projection;
};


class Drawable
{
public:
	virtual void draw() = 0;
};


class Scene : public Drawable
{
public:

	virtual void insert(Drawable* d) = 0;
	virtual void erase(Drawable* d) = 0;

	virtual std::vector<Drawable*>& all() = 0;
};


class RenderBatch : public std::vector<Drawable*>
{

};


class RenderingPass
{
public:
	virtual ~RenderingPass() {};

	virtual void prepare(int index) {};
	virtual void draw(Viewer* viewer) = 0;
	virtual void finish() {};

	/**
	 * @brief used to identify a rendering pass for shader set selection
	 */
	int id;

	Scene* scene;
};

class Renderer
{
public:
	virtual bool is_running() = 0;
	virtual bool capture(std::string path) = 0;
	virtual void draw(Viewer* viewer, std::vector<RenderingPass*> passes) = 0;
};

}
