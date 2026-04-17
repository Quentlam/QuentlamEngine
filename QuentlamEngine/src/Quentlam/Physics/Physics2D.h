#pragma once

#include "Quentlam/Core/Base.h"
#include "Quentlam/Scene/Scene.h"

namespace Quentlam {

	class QUENTLAM_API Physics2D
	{
	public:
		static bool OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop(Scene* scene);
		static void OnUpdate(Scene* scene, Timestep ts);
	};

}
