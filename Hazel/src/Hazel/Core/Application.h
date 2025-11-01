#pragma once
#include "Hazel/Core/Base.h"
#include "Hazel/Events/Event.h"
#include "Hazel/Core/Window.h"
#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Core/LayerStack.h"
#include "Hazel/ImGui/ImGuiLayer.h"
#include "Hazel/Core/Timestep.h"

int main(int argc, char** argv);

namespace Hazel
{
	class Application
	{
	public:
		Application();
		virtual ~Application();


		virtual void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }

		inline static Application& Get() { return *s_Instance; }
	private:
		void Run();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& event);

		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;

		float m_LastFrameTime = 0.0f;
	private:
		static Application* s_Instance;
		// 通过将 main 声明为友元函数，便可以在外部通过 main 来访问私有的 Run 函数
		friend int ::main(int argc, char** argv);
	};

	// 在客户端中定义该函数
	Application* CreateApplication();
}

