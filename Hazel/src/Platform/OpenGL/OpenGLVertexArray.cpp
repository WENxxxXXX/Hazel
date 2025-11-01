#include "hzpch.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Hazel
{
	OpenGLVertexArray::OpenGLVertexArray()
	{
		HZ_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		HZ_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexbuffer)
	{
		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(vertexbuffer->GetLayout().GetElements().size(), " The Vertex Buffer Layout now is empty ! ( Fix it if you don't set or not use OpenGLVertexBuffer::SetLayout())")
		// Bind VA & VB
		glBindVertexArray(m_RendererID);
		vertexbuffer->Bind();
		// Vertex Attrib 
		const auto& layout = vertexbuffer->GetLayout();
		for (const auto& element : layout)
		{
			switch (element.Type) {
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			case ShaderDataType::Bool:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(
					m_VertexBufferIndex,
					element.Count,
					element.GLType,
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset
				);
				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			{
				for (uint32_t i = 0; i < element.Count; i++)
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(
						m_VertexBufferIndex,
						element.Count,
						element.GLType,
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(sizeof(float) * element.Count * i)
					);
					glVertexAttribDivisor(m_VertexBufferIndex, 1);//每绘制一个新实例时更新顶点属性
					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				HZ_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}
		}

		m_VertexBuffers.push_back(vertexbuffer);
	}

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexbuffer)
	{
		HZ_PROFILE_FUNCTION();

		// Bind VA & IB
		glBindVertexArray(m_RendererID);
		indexbuffer->Bind();

		m_IndexBuffer = indexbuffer;
	}


}