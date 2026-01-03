#pragma once

#include <string>

#include "Hazel/Core/Base.h"

namespace Hazel
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F,
		R32UI,
		RGBA32UI,
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void BindImageTexture(uint32_t unit = 0) const = 0;

		virtual uint32_t GetRendererID() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};



	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, ImageFormat format);
		static Ref<Texture2D> Create(const std::string& path);

		virtual const std::string& GetPath() const = 0;
	};

	class TextureBuffer : public Texture
	{
	public:
		static Ref<TextureBuffer> Create(uint32_t size, ImageFormat format);
	};

}