#pragma once

#include <GLFW/glfw3.h>

namespace seen
{

struct Positionable
{
		// Dynamic Interface
		Vec3& position();
		Vec3 left();
		Vec3 forward();
		Quat orientation();

		Positionable* position(Vec3& pos);
		Positionable* position(float x, float y, float z);

		Positionable* orientation(Quat& ori);
		void orientation(mat3x3 rot);

		void world(mat4x4 world);
		mat4x4_t& world();

		mat3x3_t normal_matrix = { {
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 },
		} };
private:
		Vec3 _position;
		Quat _orientation = QUAT_I;
		mat4x4_t _world = { {
			{ 1, 0, 0, 0 },
			{ 0, 1, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 0, 1 }
		} };
};


class Viewer : public Positionable
{
public:
	virtual Viewer* view_projection(mat4x4 vp) = 0;
	virtual Viewer* view(mat4x4 v) = 0;
	virtual Viewer* projection(mat4x4 p) = 0;

	mat4x4_t _view;
	mat4x4_t _projection;
};


class Drawable
{
public:
	virtual void draw() = 0;
};


class Scene : public Drawable
{
public:
	virtual std::vector<Drawable*>& drawables() = 0;
};

class RenderBatch : public std::vector<Drawable*>
{

};

class RenderingPass
{
public:
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
