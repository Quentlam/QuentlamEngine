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

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { return m_ColorAttachments[index]; }
		virtual uint32_t GetDepthAttachmentRendererID() const override { return m_DepthAttachment; }
		
		const FrameBufferSpecification& GetSpecification() const override { return m_Specification; };


	private:
		uint32_t m_RendererID = 0;
		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		FrameBufferSpecification m_Specification;


	};

};



