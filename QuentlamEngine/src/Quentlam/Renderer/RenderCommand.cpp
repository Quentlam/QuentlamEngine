#include "qlpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Quentlam
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();


}
