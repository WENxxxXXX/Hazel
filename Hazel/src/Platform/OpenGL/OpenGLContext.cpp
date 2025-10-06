#include "hzpch.h"
#include "OpenGLContext.h"

#include <glad/glad.h>// ---- glad is OpenGL specific ----
#include "GLFW/glfw3.h"

namespace Hazel
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		:m_WindowHandle(windowHandle)
	{
		HZ_CORE_ASSERT(windowHandle, "Window handle is null ! ( But it shouldn't be )")
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		//通过glad加载OpenGL提供的各种图形渲染函数
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HZ_CORE_ASSERT(status, "Failed to initialize Glad!");

		HZ_CORE_INFO("OpenGL info: ");
		HZ_CORE_INFO("	Vendor: {0}", (const char*)glGetString(GL_VENDOR));
		HZ_CORE_INFO("	Renderer: {0}", (const char*)glGetString(GL_RENDERER));
		HZ_CORE_INFO("	Version: {0}", (const char*)glGetString(GL_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}