#pragma once

#include "core.h"

namespace seen
{
	struct Light
	{
		Vec3 position, direction;
		Vec3 power;
		float ambience;
		mat4x4_t projection;
	};
}
