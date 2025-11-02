#pragma once

#include "Hazel/Renderer/FrameBuffer.h"

namespace Hazel
{

	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		void Recreate();

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;

		uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; };
		const FrameBufferSpecification& GetSpecification() const override { return m_Specification; };
	private:
		uint32_t m_RendererID = 0;

		uint32_t m_ColorAttachment = 0;
		uint32_t m_BufferAttachment = 0;
		FrameBufferSpecification m_Specification;
	};

}