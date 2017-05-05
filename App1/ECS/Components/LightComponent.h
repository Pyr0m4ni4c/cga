#pragma once

#include "..\Component.h"
#include "..\..\ILight.h"

namespace ECS { namespace Components {
	class LightComponent : public Component
	{
	public:
		LightComponent(ILight * light) : light(light) { }
		ILight * light;
	};
}}
