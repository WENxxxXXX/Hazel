#pragma once

#include <glm/glm.hpp>

#include "Hazel/Renderer/VertexArray.h"

namespace Hazel
{

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1, DirectX = 2
		};

		enum class BlendFactor
		{
			Zero = 0,
			One,
			SrcColor,
			OneMinusSrcColor,
			SrcAlpha,
			OneMinusSrcAlpha,
			DstAlpha,
			OneMinusDstAlpha,
			DstColor,
			OneMinusDstColor,
			ConstantColor,
			OneMinusConstantColor,
			ConstantAlpha,
			OneMinusConstantAlpha
		};
	public:
		virtual ~RendererAPI() = default;

		static Scope<RendererAPI> Create();

		virtual void Clear() = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;

		virtual void Init() = 0;
		virtual std::pair<int, int> GetMaxMonitorSize() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

		virtual void Flush() = 0;

		virtual void SetBlendFunci(uint32_t buf, BlendFactor src, BlendFactor dst) = 0;
		virtual void SetColorMaski(uint32_t buf, bool r, bool g, bool b, bool a) = 0;
		virtual void SetBlendFunc(BlendFactor src, BlendFactor dst) = 0;
		virtual void SetDepthTest(bool enabled) = 0;
		virtual void SetDepthMask(bool enabled) = 0;

		virtual void BindTexture(uint32_t rendererID, uint32_t slot) = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;

		virtual void MemoryBarrierTexFetch() = 0;

		inline static API GetAPI() { return s_API; }
		inline static API SetAPI(API api) { s_API = api; }

	private:
		static API s_API;
	};

}