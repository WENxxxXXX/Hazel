#include "hzpch.h"

#include <GLFW/glfw3.h>

#include "Application.h"

#include "Hazel/Core/Log.h"
#include "Hazel/Core/Input.h"

#include "Hazel/Renderer/Renderer.h"

namespace Hazel
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		HZ_PROFILE_FUNCTION();

		HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window = Window::Create();
		m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));//���������д��OnEvent��������������Ϊ׼
		
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

	void Application::OnEvent(Event& e)//Application�д�������ʱ�����˺�����Ϊ�ص�����
	{
		HZ_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(Application::OnWindowResize));

		//HZ_CORE_TRACE("{0}", e.ToString());

		//ͼ����¼������Ƿ���ģ���β��ͷ��������������������е� iter ��Ҫʹ��ǰ�õ������ȼӺ��ã�
		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)	
		{
			(*it)->OnEvent(e);
			if (e.Handled)
			{
				break;
			}
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

					for (Layer* layer : m_LayerStack)//����ͼ��
						layer->OnUpdate(timestep);//ִ��ͼ���߼�����(����Ӧ�ó�����߼�״̬��
				}
				m_ImGuiLayer->Begin();
				{
					HZ_PROFILE_SCOPE("LayerStack OnImGuiRender");
					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();//����ͼ��ʵ����Ⱦ�������߼����º���ܽ��е���Ⱦ������
				}
				m_ImGuiLayer->End();
			}
			m_Window->OnUpdate();//���´���
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

