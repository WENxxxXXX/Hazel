#pragma once

#include "Hazel/Renderer/FrameBuffer.h"

namespace Hazel
{

	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		void Resize();

		void Bind() override;
		void Unbind() override;

		uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; };
		const FrameBufferSpecification& GetSpecification() const override { return m_Specification; };
	private:
		uint32_t m_RendererID;

		uint32_t m_ColorAttachment;
		uint32_t m_BufferAttachment;
		FrameBufferSpecification m_Specification;
	};

}