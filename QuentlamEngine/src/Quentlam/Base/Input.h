#pragma once

#include "Quentlam/Base/Core.h"
#include "Quentlam/Base/KeyCodes.h"
#include "Quentlam/Base/MouseButtonCodes.h"

namespace Quentlam
{
	class QUENTLAM_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);//键盘是否按下

		static bool IsMouseButtonPressed(MouseCode button);//鼠标是否按下
		static std::pair<float, float> GetMousePosition();//返回鼠标位置
		static float GetMouseX();//返回鼠标位置X
		static float GetMouseY();//返回鼠标位置Y
	};

} 