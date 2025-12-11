#include "hzpch.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Hazel
{

	
	//������Ϣ�ص�����(OpenGLMessageCallback)����
	void OpenGLMessageCallback
	(unsigned source, unsigned type, unsigned id, unsigned severity, int length, const char* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         HZ_CORE_CRIRICAL(message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:       HZ_CORE_ERROR(message); return;
		case GL_DEBUG_SEVERITY_LOW:          HZ_CORE_WARN(message); return;
		case GL_DEBUG_SEVERITY_NOTIFICATION: HZ_CORE_TRACE(message); return;
		}

		HZ_CORE_ASSERT(false, "Unknown severity level!");
	}

	GLenum OpenGLBlendFactorToGL(RendererAPI::BlendFactor factor)
	{
		switch (factor)
		{
		case RendererAPI::BlendFactor::Zero: return GL_ZERO;
		case RendererAPI::BlendFactor::One: return GL_ONE;
		case RendererAPI::BlendFactor::SrcColor: return GL_SRC_COLOR;
		case RendererAPI::BlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
		case RendererAPI::BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
		case RendererAPI::BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
		case RendererAPI::BlendFactor::DstAlpha: return GL_DST_ALPHA;
		case RendererAPI::BlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
		case RendererAPI::BlendFactor::DstColor: return GL_DST_COLOR;
		case RendererAPI::BlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
		case RendererAPI::BlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
		case RendererAPI::BlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
		case RendererAPI::BlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
		case RendererAPI::BlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
		}
		return 0;
	}

	void OpenGLRendererAPI::Init()
	{
		HZ_PROFILE_FUNCTION();

#ifdef HZ_DEBUG
		//����OpenGL���ɵĵ�����Ϣ�����ɺ͹���
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		//glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
	}

	std::pair<int, int> OpenGLRendererAPI::GetMaxMonitorSize()
	{
		std::pair<int, int> res;
		int monitorCount = 0;
		GLFWmonitor** pMonitor = glfwGetMonitors(&monitorCount);
		for (int i = 0; i < monitorCount; ++i)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(pMonitor[i]);
			res.first = std::max(res.first, mode->width);
			res.second = std::max(res.second, mode->height);
		}
		return res;
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::Flush()
	{
		glFlush();
	}
	
	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::SetBlendFunci(uint32_t buf, RendererAPI::BlendFactor src, RendererAPI::BlendFactor dst)
	{
		glBlendFunci(buf, OpenGLBlendFactorToGL(src), OpenGLBlendFactorToGL(dst));
	}

	void OpenGLRendererAPI::SetColorMaski(uint32_t buf, bool r, bool g, bool b, bool a)
	{
		glColorMaski(buf, r, g, b, a);
	}

	void OpenGLRendererAPI::SetBlendFunc(RendererAPI::BlendFactor src, RendererAPI::BlendFactor dst)
	{
		glBlendFunc(OpenGLBlendFactorToGL(src), OpenGLBlendFactorToGL(dst));
	}

	void OpenGLRendererAPI::SetDepthTest(bool enabled)
	{
		if (enabled)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
	}

	void OpenGLRendererAPI::SetDepthMask(bool enabled)
	{
		glDepthMask(enabled ? GL_TRUE : GL_FALSE);
	}

	void OpenGLRendererAPI::BindTexture(uint32_t rendererID, uint32_t slot)
	{
		glBindTextureUnit(slot, rendererID);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray)
	{
		vertexArray->Bind();
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);//��Ⱦ��֮�󣬽��ǰ����������Ӱ�쵽����������������Ⱦ
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();

		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLRendererAPI::MemoryBarrierTexFetch()
	{
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
	}

	// Cherno do:
	//void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount /*= 0*/){
	//	uint32_t count = indexCount ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
	//	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);}


}