#pragma once

#include "core.h"

namespace seen
{
	struct Light
	{
        Light(Vec3 power, float ambience, float near=0.1, float far=100)
        {
            this->power = power;
            this->ambience = ambience;
            mat4x4_perspective(
                this->projection.v,
                M_PI / 2,
                1,
                near,
                far
            );
        };

		Vec3 position, direction;
		Vec3 power;
		float ambience;
		mat4x4_t projection;
	};
}
