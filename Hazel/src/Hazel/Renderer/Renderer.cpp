#include "hzpch.h"
#include "Renderer.h"

namespace Hazel
{
	//静态变量需要 1.在类外 2.被初始化
	RendererAPI Renderer::s_API = RendererAPI::OpenGL;
}