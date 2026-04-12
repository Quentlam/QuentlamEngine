#include "qlpch.h"
#include "OpenGLFrameBuffer.h"
#include "Quentlam/Base/Core.h"

#include <glad/glad.h>

namespace Quentlam
{
	OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification& spec)
		: m_Specification(spec)
	{
		Invalidate();
	}
	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_ColorAttachment);
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFrameBuffer::Invalidate()
	{
		if(m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_ColorAttachment);
			glDeleteTextures(1, &m_DepthAttachment);
		}
		
		glCreateFramebuffers(1, &m_RendererID);					//生成一个帧缓冲对象，并把其句柄存放在 m_RendererID。
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);		//绑定该帧缓冲对象，后续对帧缓冲的操作都作用于它。

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment); //生成一个纹理对象，类型为 2D 纹理，句柄存放在 m_ColorAttachment。
		glBindTexture(GL_TEXTURE_2D, m_ColorAttachment); //绑定该纹理对象，后续对纹理的操作都作用于它。
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);//为纹理分配存储。
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//设置纹理在缩小滤波时使用线性过滤。
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//设置纹理在放大滤波时使用线性过滤。

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0);
		//将纹理附着到当前绑定的帧缓冲对象的颜色附件 0（level 为 0）。


		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment); //生成一个纹理对象，类型为 2D 纹理，句柄存放在 m_ColorAttachment。
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachment); //绑定该纹理对象，后续对纹理的操作都作用于它。
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height);//为深度模板纹理分配存储。
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0);

		QL_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);//解绑帧缓冲，恢复默认帧缓冲。
	}

	void OpenGLFrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFrameBuffer::UnBind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > 8192 || height > 8192)
		{
			QL_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();

	}

};