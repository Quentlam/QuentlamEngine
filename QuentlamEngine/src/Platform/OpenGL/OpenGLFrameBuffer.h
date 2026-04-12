#pragma once
#include "Quentlam/Renderer/FrameBuffer.h" 

namespace Quentlam
{
	class OpenGLFrameBuffer : public FrameBuffer
	{

	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec = FrameBufferSpecification());
		virtual ~OpenGLFrameBuffer();
		
		void Invalidate();

	    void Bind()const override;
		void UnBind()const override;
		
		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }
		
		const FrameBufferSpecification& GetSpecification() const override { return m_Specification; };


	private:
		uint32_t m_RendererID = 0;
		uint32_t m_ColorAttachment = 0;
		uint32_t m_DepthAttachment = 0;

		FrameBufferSpecification m_Specification;


	};

};



