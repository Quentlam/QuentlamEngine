#include "qlpch.h"
#include <GLFW/glfw3.h>

#include "Quentlam/Core/KeyCodes.h"
#include "Quentlam/Core/MouseButtonCodes.h"
#include "Quentlam/Core/Input.h"
#include "Quentlam/Core/Application.h"


namespace Quentlam
{
	bool Input::IsKeyPressed(KeyCode keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey((GLFWwindow*)window,keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}


	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton((GLFWwindow*)window, button);
		return state == GLFW_PRESS;
	}
	std::pair<float, float> Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos,(float)ypos };
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}

}
