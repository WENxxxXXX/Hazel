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
	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			HZ_CORE_ASSERT((index < Count), "Application Command Line Arags invalid");
			return Args[index];
		}
	};

	class Application
	{
	public:
		Application(const std::string& name = "Nut App", 
			ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
		virtual ~Application();


		virtual void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		inline Window& GetWindow() { return *m_Window; }
		inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
		inline static Application& Get() { return *s_Instance; }
		inline ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		// 不同于 OnWindowClose ，WindowClose 是公有的关闭窗口函数，而 OnWindowClose 是回调函数中的事件处理
		inline void WindowClose() { m_Running = false; }
	private:
		void Run();

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& event);

	private:
		ApplicationCommandLineArgs m_CommandLineArgs;

		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;

		float m_LastFrameTime = 0.0f;

		static Application* s_Instance;
		// 通过将 main 声明为友元函数，便可以在外部通过 main 来访问私有的 Run 函数
		friend int ::main(int argc, char** argv);
	};

	// 在客户端中定义该函数
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

