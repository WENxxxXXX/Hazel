#pragma once

#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel
{

	class RendererCommand
	{
	public:
		// static variable frmm Create(), which returns virtual funcs already overrided in Platform
		// So bind the functions form corresponding interface in OpenGL/DirectX/Metal/Valkan
		
		
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static std::pair<int, int> GetMaxMonitorSize()
		{
			return s_RendererAPI->GetMaxMonitorSize();
		}
		
		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void Flush()
		{
			s_RendererAPI->Flush();
		}
		
		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void SetBlendFunci(uint32_t buf, RendererAPI::BlendFactor src, RendererAPI::BlendFactor dst)
		{
			s_RendererAPI->SetBlendFunci(buf, src, dst);
		}

		inline static void SetColorMaski(uint32_t buf, bool r, bool g, bool b, bool a)
		{
			s_RendererAPI->SetColorMaski(buf, r, g, b, a);
		}

		inline static void SetBlendFunc(RendererAPI::BlendFactor src, RendererAPI::BlendFactor dst)
		{
			s_RendererAPI->SetBlendFunc(src, dst);
		}

		inline static void SetDepthTest(bool enabled)
		{
			s_RendererAPI->SetDepthTest(enabled);
		}

		inline static void SetDepthMask(bool enabled)
		{
			s_RendererAPI->SetDepthMask(enabled);
		}

		inline static void BindTexture(uint32_t rendererID, uint32_t slot)
		{
			s_RendererAPI->BindTexture(rendererID, slot);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray)//GL_STATIC_DRAW
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)//GL_DYNAMIC_DRAW(with batch rendering)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		inline static void MemeryBarrierTexFetch()
		{
			s_RendererAPI->MemoryBarrierTexFetch();
		}

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}