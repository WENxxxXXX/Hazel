#include "hzpch.h"

#include <GLFW/glfw3.h>

#include "Application.h"

#include "Hazel/Core/Log.h"
#include "Hazel/Core/Input.h"

#include "Hazel/Renderer/Renderer.h"

namespace Hazel
{
	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name /* = Nut App */, ApplicationCommandLineArgs args /* = ApplicationCommandLineArgs()*/)
		:m_CommandLineArgs(args)
	{
		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create(WindowProps(name));
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));//如果子类重写了OnEvent函数，则以子类为准
		
		Renderer::Init();
		
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}
	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		HZ_PROFILE_FUNCTION();

		m_LayerStack.PushOverLay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)//Application中创建窗口时，将此函数设为回调函数
	{
		HZ_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(Application::OnWindowResize));

		//HZ_CORE_TRACE("{0}", e.ToString());

		//图层的事件处理是反向的（从尾到头），！！！反向迭代器中的 iter 需要使用前置递增（先加后用）
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)	
		{
			if (e.Handled)
			{
				break;
			}
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		HZ_PROFILE_FUNCTION();

		while (m_Running)
		{
			HZ_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					HZ_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : m_LayerStack)//更新图层
						layer->OnUpdate(timestep);//执行图层逻辑更新(更新应用程序的逻辑状态）
				}
				m_ImGuiLayer->Begin();
				{
					HZ_PROFILE_SCOPE("LayerStack OnImGuiRender");
					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();//进行图层实际渲染操作（逻辑更新后才能进行的渲染操作）
				}
				m_ImGuiLayer->End();
			}
			m_Window->OnUpdate();//更新窗口
		}
	}
	
	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& event)
	{
		HZ_PROFILE_FUNCTION();

		m_Minimized = false;
		Renderer::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;

		if (event.GetWidth() == 0 && event.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}
	}
}

