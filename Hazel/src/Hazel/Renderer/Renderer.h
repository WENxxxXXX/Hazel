#pragma once

namespace Hazel
{
	enum class RendererAPI
	{
		None = 0, OpenGL = 1, DirectX = 2
	};

	class Renderer
	{
	public:
		static inline RendererAPI SetAPI(RendererAPI api) { s_API = api; }
		//ע�⾲̬����ʹ��ʱ���������ʶ
		static inline RendererAPI GetAPI() { return s_API; }
	private:
		static RendererAPI s_API;
	};
}