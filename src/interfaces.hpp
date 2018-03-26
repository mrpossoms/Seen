#pragma once

#include <GLFW/glfw3.h>

namespace seen
{

class Positionable
{
	public:
		// Dynamic Interface
		virtual Vec3 position() = 0;
		virtual Positionable* position(Vec3& pos) = 0;
		virtual Positionable* position(float x, float y, float z) = 0;

		virtual Quat orientation() = 0;
		virtual Positionable* orientation(Quat& ori) = 0;

		virtual void rotation(mat3x3 rot) = 0;
		virtual void matrix(mat4x4 world) = 0;
};


class Viewer : public Positionable
{
public:
	virtual Viewer* view_projection(mat4x4 vp) = 0;
	virtual Viewer* view(mat4x4 v) = 0;
	virtual Viewer* projection(mat4x4 p) = 0;

	mat4x4 _view;
	mat4x4 _projection;
};


class Drawable
{
public:
	virtual void draw(Viewer* viewer) = 0;
};


class Scene : public Drawable
{
public:
	virtual std::vector<Drawable*>& drawables() = 0;
};

class RenderingPass : public Drawable
{
public:
	virtual void prepare() {};
	virtual void draw(Viewer* viewer) {};
	virtual void draw(Viewer* viewer, Scene* scene, std::vector<Drawable*>& excluding) = 0;
	virtual void finish() {};

	/**
	 * @brief used to identify a rendering pass for shader set selection
	 */
	int id;
};

class Renderer : public RenderingPass
{
public:
	virtual bool is_running() = 0;
	virtual bool capture(std::string path) = 0;
};

}
