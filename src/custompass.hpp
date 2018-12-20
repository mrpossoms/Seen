#pragma once

#include "core.h"
#include "listscene.hpp"
#include "cubemap.hpp"
#include "light.hpp"

namespace seen
{

class CustomPass : public RenderingPass
{
public:
	CustomPass();
	CustomPass(std::function<void (int)> prep);
	~CustomPass();

	void prepare(int index) override;
	void draw(Viewer* viewer) override;
	void finish() override {}

	int instances;

	std::function<void (int)> preparation_function;
};

class ShadowPass : public RenderingPass
{
	friend struct ShaderProgram;

public:
	ShadowPass(int resolution=1024, bool generate_mipmaps=true);
	~ShadowPass();

	void prepare(int index) override;
	void draw(Viewer* viewer) override;
	void finish() override;

	// Scene* scene;
	std::vector<const Light*> lights;

private:
	Cubemap* _cubemap;
	bool _generate_mipmaps;
};

}
