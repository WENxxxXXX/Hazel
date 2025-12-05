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
		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		void ClearAttachment(uint32_t attachmentIndex, int value) override;
		void ClearAttachment(uint32_t attachmentIndex, float value) override;
		void ClearAttachment(uint32_t attachmentIndex, const glm::vec4& value) override;

		uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override 
		{ 
			HZ_CORE_ASSERT((index < m_ColorAttachmentIDs.size()), 
				"Index should be less than the number of all Color attachments");
			return m_ColorAttachmentIDs[index];
		};
		const FrameBufferSpecification& GetSpecification() const override { return m_Specification; };
	private:
		uint32_t m_RendererID = 0;

		uint32_t m_ColorAttachment = 0;
		uint32_t m_BufferAttachment = 0;
		FrameBufferSpecification m_Specification;

		std::vector<FrameBufferAttachmentFormat> m_ColorAttachmentSpecifications;
		FrameBufferAttachmentFormat m_BufferAttachmentSpecification;

		std::vector<uint32_t> m_ColorAttachmentIDs;
		uint32_t m_BufferAttachmentID = 0;
	};

}