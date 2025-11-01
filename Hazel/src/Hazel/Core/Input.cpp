#include "hzpch.h"
#include "Hazel/Core/Input.h"

#ifdef HZ_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsInput.h"
#endif

namespace Hazel {

	Scope<Input> Input::s_Instance = Input::Create();//s_Instance ��һ��û�а����κ����ݵ�ʵ������

	Scope<Input> Input::Create()
	{
		#ifdef HZ_PLATFORM_WINDOWS
			return CreateScope<WindowsInput>();
		#elif defined HZ_PLATFORM_MACOS
			return CreateScope<MacosInput>();
		#elif defined HZ_PLATFORM_LINUX
			return CreateScope<LinuxInput>();
		#else
			HZ_CORE_ASSERT(false, "Unknown platform!");
			return nullptr;
		#endif
	}

}