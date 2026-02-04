#pragma once

namespace Quentlam
{
	class QUENTLAM_API GraphicsContext
	{
	public:
		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;



	};
}