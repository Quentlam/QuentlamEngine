#pragma once


namespace Quentlam
{

	struct FrameBufferSpecification
	{
		uint32_t Width, Height;
		bool SwapChainTarget = false;//是否为交换链目标的标志位
	};



	class FrameBuffer
	{
	public:
		virtual void Bind()const = 0;
		virtual void UnBind()const = 0;

		virtual uint32_t GetColorAttachmentRendererID() const = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);


	};


}

