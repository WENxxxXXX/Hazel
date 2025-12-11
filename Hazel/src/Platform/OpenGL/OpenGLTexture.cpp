#include "hzpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include "stb_image.h"

namespace Hazel
{
	// !!! This Function needs to be used with "SetData" !!!
	// �������������Ĵ洢�ռ䣬û�ж������洢ֵ
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height)
	{
		HZ_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		// ��������(width �� height ��ʾ���ص�����γɵĿ���ߣ�
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//��������
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);//��������ģʽ
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, ImageFormat format)
		:m_Width(width), m_Height(height)
	{
		HZ_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		switch (format)
		{
		case ImageFormat::R8:
			m_InternalFormat = GL_R8;
			m_DataFormat = GL_RED;
			break;
		case ImageFormat::RGB8:
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
			break;
		case ImageFormat::RGBA8:
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
			break;
		case ImageFormat::RGBA32F:
			m_InternalFormat = GL_RGBA32F;
			m_DataFormat = GL_RGBA;
			break;
		case ImageFormat::R32UI:
			m_InternalFormat = GL_R32UI;
			m_DataFormat = GL_RED_INTEGER;
			break;
		case ImageFormat::RGBA32UI:
			m_InternalFormat = GL_RGBA32UI;
			m_DataFormat = GL_RGBA_INTEGER;
			break;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		:m_Path(path)
	{
		HZ_PROFILE_FUNCTION();

		stbi_set_flip_vertically_on_load(1);

		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			HZ_PROFILE_SCOPE("stbi_load -> OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		HZ_CORE_ASSERT(data, "Failed to load image!");

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;										// �����ڲ������ʽ �� ���ݵ��ϴ���ʽ
		if (channels == 3) {
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (channels == 4) {
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		HZ_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!")

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);//gl func need unsigned int data So we assign the value of width to m_Width

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);//��������ģʽ
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, 
			dataFormat, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)// ���ݺ������ڴ��С
	{
		HZ_PROFILE_FUNCTION();

		uint32_t bpp = (m_DataFormat == GL_RGBA ? 4 : 3);
		HZ_CORE_ASSERT((size == m_Width * m_Height * bpp), "Data must contain the full texture! Please check that the size of the data matches the format of the data");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);// �ϴ�����
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		HZ_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::BindImageTexture(uint32_t unit) const
	{
		HZ_PROFILE_FUNCTION();

		glBindImageTexture(unit, m_RendererID, 0, GL_TRUE, 0, GL_READ_WRITE, m_InternalFormat);
	}

	OpenGLTextureBuffer::OpenGLTextureBuffer(uint32_t size, ImageFormat format)
		: m_Size(size)
	{
		HZ_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;

		switch (format)
		{
		case ImageFormat::R8:
			m_InternalFormat = GL_R8;
			break;
		case ImageFormat::RGB8:
			m_InternalFormat = GL_RGB8;
			break;
		case ImageFormat::RGBA8:
			m_InternalFormat = GL_RGBA8;
			break;
		case ImageFormat::RGBA32F:
			m_InternalFormat = GL_RGBA32F;
			break;
		case ImageFormat::R32UI:
			m_InternalFormat = GL_R32UI;
			break;
		case ImageFormat::RGBA32UI:
			m_InternalFormat = GL_RGBA32UI;
			break;
		}

		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_TEXTURE_BUFFER, m_BufferID);
		glBufferData(GL_TEXTURE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		glCreateTextures(GL_TEXTURE_BUFFER, 1, &m_RendererID);
		glTextureBuffer(m_RendererID, m_InternalFormat, m_BufferID);
	}

	OpenGLTextureBuffer::~OpenGLTextureBuffer()
	{
		HZ_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
		glDeleteBuffers(1, &m_BufferID);
	}

	void OpenGLTextureBuffer::SetData(void* data, uint32_t size)
	{
		glBindBuffer(GL_TEXTURE_BUFFER, m_BufferID);
		glBufferSubData(GL_TEXTURE_BUFFER, 0, size, data);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}

	void OpenGLTextureBuffer::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTextureBuffer::BindImageTexture(uint32_t unit) const
	{
		glBindImageTexture(unit, m_RendererID, 0, GL_FALSE, 0, GL_READ_WRITE, m_InternalFormat);
	}

}