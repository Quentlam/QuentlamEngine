#pragma once

#include "Quentlam/Core/Core.h"

namespace Quentlam
{
	class QUENTLAM_API Input
	{
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }//键盘是否按下

		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }//鼠标是否按下
		inline static std::pair<float,float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }//返回鼠标位置
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }//返回鼠标位置X
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }//返回鼠标位置Y


	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
	private:
		static Input* s_Instance;


	};

}