#pragma once
#include "qlpch.h"
#include "Quentlam/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Quentlam
{
	class QUENTLAM_API OpenGLContext:public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);
		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;

	};
}

