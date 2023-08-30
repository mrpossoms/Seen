#pragma once

#include "core.h"

namespace seen
{
	struct Light
	{
		vec<3> position, direction;
		vec<3> power;
		float ambience;
		mat<4,4> projection;
	};
}
