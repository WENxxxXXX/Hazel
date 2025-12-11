#include "hzpch.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

#include "glad/glad.h"

namespace Hazel
{
	//////////////////////////////////////////////////////
	/////////////////// VertexBuffer /////////////////////
	//////////////////////////////////////////////////////

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) 
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetData(const void* data, uint32_t size) const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	//////////////////////////////////////////////////////
	/////////////////// IndexBuffer //////////////////////
	//////////////////////////////////////////////////////

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		:m_Count(count)
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}


	//////////////////////////////////////////////////////
	/////////////////// PixelUnpackBuffer //////////////////////
	//////////////////////////////////////////////////////

	OpenGLPixelUnpackBuffer::OpenGLPixelUnpackBuffer(uint32_t size)
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_RendererID);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, size * sizeof(uint32_t), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	OpenGLPixelUnpackBuffer::~OpenGLPixelUnpackBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLPixelUnpackBuffer::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_RendererID);
	}

	void OpenGLPixelUnpackBuffer::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}

	void* OpenGLPixelUnpackBuffer::MapBuffer() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_RendererID);
		return glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	}

	void OpenGLPixelUnpackBuffer::UnmapBuffer() const
	{
		HZ_PROFILE_FUNCTION();

		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}


	//////////////////////////////////////////////////////
	/////////////////// OpenGLAtomicCounterBuffer ////////
	//////////////////////////////////////////////////////
	OpenGLAtomicCounterBuffer::OpenGLAtomicCounterBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_RendererID);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
		glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), NULL, GL_DYNAMIC_COPY);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	}

	OpenGLAtomicCounterBuffer::~OpenGLAtomicCounterBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLAtomicCounterBuffer::Bind() const
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
	}

	void OpenGLAtomicCounterBuffer::BindBase(uint32_t index) const
	{
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, index, m_RendererID);
	}

	void OpenGLAtomicCounterBuffer::Unbind() const
	{
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	}

	void* OpenGLAtomicCounterBuffer::MapBuffer() const
	{
		HZ_PROFILE_FUNCTION();

		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
		return glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
	}

	void OpenGLAtomicCounterBuffer::UnmapBuffer() const
	{
		HZ_PROFILE_FUNCTION();

		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	}

	void OpenGLAtomicCounterBuffer::Reset(uint32_t value) const
	{
		HZ_PROFILE_FUNCTION();
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_RendererID);
		uint32_t* ptr = (uint32_t*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);
		ptr[0] = value;
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	}

}