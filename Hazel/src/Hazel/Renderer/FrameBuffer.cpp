#include "hzpch.h"

#include "Hazel/Renderer/Renderer.h"
#include "Hazel/Renderer/Framebuffer.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

namespace Hazel
{

	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::None: HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported! ")
			return nullptr;
		case RendererAPI::API::OpenGL:
			return CreateRef<OpenGLFrameBuffer>(spec);
		case RendererAPI::API::DirectX: HZ_CORE_ASSERT(false, "RendererAPI::DirectX is currently not supported! ")
			return nullptr;
		}

		HZ_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}

}