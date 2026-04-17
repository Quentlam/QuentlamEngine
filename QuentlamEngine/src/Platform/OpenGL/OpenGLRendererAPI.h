#pragma once
#include "qlpch.h"
#include "Quentlam/Renderer/RendererAPI.h"


namespace Quentlam
{
	class OpenGLRendererAPI :public RendererAPI
	{
	public:
		virtual void Init() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void SetStencilTest(bool enable) override;
		virtual void SetStencilFunc(uint32_t func, int ref, uint32_t mask) override;
		virtual void SetStencilOp(uint32_t fail, uint32_t zfail, uint32_t zpass) override;
		virtual void SetStencilMask(uint32_t mask) override;
		virtual void SetDepthTest(bool enable) override;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

	};


}