#pragma once

#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel 
{

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Init() override;
		std::pair<int, int> GetMaxMonitorSize() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void Flush() override;
		
		void Clear() override;
		void SetClearColor(const glm::vec4& color) override;

		void SetBlendFunci(uint32_t buf, RendererAPI::BlendFactor src, RendererAPI::BlendFactor dst) override;
		void SetColorMaski(uint32_t buf, bool r, bool g, bool b, bool a) override;
		void SetBlendFunc(RendererAPI::BlendFactor src, RendererAPI::BlendFactor dst) override;
		void SetDepthTest(bool enabled) override;
		void SetDepthMask(bool enabled) override;

		void BindTexture(uint32_t rendererID, uint32_t slot) override;

		void DrawIndexed(const Ref<VertexArray>& vertexArray) override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;

		void MemoryBarrierTexFetch() override;
	};

}