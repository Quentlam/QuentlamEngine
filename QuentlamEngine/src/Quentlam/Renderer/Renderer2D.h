#pragma once
#include "Renderer.h"
#include "OrthographicCamera.h"
#include "Texture.h"
#include "SubTexture2D.h"

namespace Quentlam
{
	class QUENTLAM_API Renderer2D : Renderer
	{

	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(OrthographicCamera& camera);
		static void EndScene();
		static void Flush();


		//primitive (基本)
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));




		//Rotation is in radians (弧度制旋转几何体)
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float rotation, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const float rotation, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const float rotation, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const float rotation, const float tilingFactor = 1.0f, const glm::vec4& tinColor = glm::vec4(1.0f));




		//Statistic
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; };
			uint32_t GetTotalIndexCount() { return QuadCount * 3; };
		};


		static void ResetStats();
		static Statistics GetStatistics();

	
	private:
		static void FlushAndReset();

	};

}


