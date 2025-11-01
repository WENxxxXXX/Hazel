#include "hzpch.h"
#include "Hazel/Core/Window.h"

#ifdef HZ_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

namespace Hazel {

	Scope<Window> Window::Create(const WindowProps& props)
	{
		#ifdef HZ_PLATFORM_WINDOWS
			return CreateScope<WindowsWindow>(props);
		#elif defined HZ_PLATFORM_MACOS
			return CreateScope<MacosWindow>(props);
		#elif defined HZ_PLATFORM_LINUX
			return CreateScope<LinuxWindow>(props);
		#else
			HZ_CORE_ASSERT(false, "Unknown platform!");
			return nullptr;
		#endif
	}


}