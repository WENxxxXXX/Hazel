#include <Hazel.h>
#include "Hazel/Core/EntryPoint.h"
#include "Sandbox2D.h"
#include "ExampleLayer.h"

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		//取消在 sandbox中 PushOverlay(new Nut::ImGuiLayer()); ，
		// 将其作为 Hazel 运行时自动添加的图层:DemoWindow
		// （在 application.cpp 中固定调用 OnImGuiRender 函数）
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{

	}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}