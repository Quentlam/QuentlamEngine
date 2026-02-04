#pragma once
#include "Renderer.h"
#include "OrthographicCamera.h"

namespace Quentlam
{
	class QUENTLAM_API Renderer2D : Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();



		static void DrawQuad(const glm::vec2& position,const glm::vec2& size,const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
	
	
	private:

	};

}


