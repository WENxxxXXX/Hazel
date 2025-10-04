--[[  ����premake�е�token�����ǣ��� https://premake.github.io/docs/Tokens/  ]]

workspace "Hazel"
    architecture "x64"
    configurations 
    { 
        "Debug",
        "Release",
        "Dist" 
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"--������������м�Ŀ¼��д��

-- ������Ը�Ŀ¼���������Ŀ¼����Ŀ¼
-- ����ȫ�ֱ������������ʹ��
IncludeDir = {}--����һ����ΪIncludeDir�ı�
IncludeDir["GLFW"] = "Hazel/vendor/GLFW/include"--�����"GLFW"����������·��

include "Hazel/vendor/GLFW"-- �������Ŀ¼�µ�lua�ļ���������Щlua�ļ�������Ŀ

project "Hazel"
    location "Hazel"        --��Ŀ�ļ������Ŀ¼
    kind "SharedLib"        --���ͣ���̬�⣩
    language "C++"          --����

    targetdir  ("bin/" .. outputdir .. "/%{prj.name}")      --���Ŀ¼(.. XX ..�� ".."���ַ������ӷ�)
    objdir  ("bin-int/" .. outputdir .. "/%{prj.name}")     --�м�Ŀ¼

    pchheader "hzpch.h"     --Ԥ����ͷ�ļ�
    pchsource "Hazel/src/hzpch.cpp"--VS��Ҫ���Դ�ļ�

    files       --��Դ�����ļ���ӵ��ض�����Ŀ��
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs --����Ŀ¼
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}"-- %{}ʹ��ȫ�ֱ���
    }

    links       --Ϊ��Ŀ(.dll)����������
    {
        "GLFW", --����GLFW��Ŀ
        "opengl32.lib"
    }

    filter "system:windows"     --������(���ϵͳ��windows)
        cppdialect "C++17"      --C++��׼������ʱ��
        staticruntime "On"      --�Ƿ�̬��������ʱ�⣨dll���Ե��ļ���Ҫ�򿪣�
        systemversion "latest"  --windows SDK �汾
        defines                 --�������
        {
            "HZ_PLATFORM_WINDOWS",
            "HZ_BUILD_DLL"
        }
        outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" 
        postbuildcommands       --������Ŀ��ɺ�ִ�е�ָ��
        {
            ("{MKDIR} ../bin/" .. outputdir .. "/Sandbox/"),
            ("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox/%{cfg.buildtarget.name}")
        }

        buildoptions "/utf-8"

    filter "configurations:Debug"
        defines "HZ_DEBUG"
        buildoptions "/MDd"
        symbols "On"

    filter "configurations:Release"
        defines "HZ_RELEASE"
        buildoptions "/MD"
        optimize "On"

    filter "configurations:Dist"
        defines "HZ_DIST"
        buildoptions "/MD"
        optimize "On"


project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    targetdir  ("bin/" .. outputdir .. "/%{prj.name}")
    objdir  ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }
    includedirs
    {
        "Hazel/vendor/spdlog/include",
        "Hazel/src"
    }

    links
    {
        "Hazel"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"
        defines
        {
            "HZ_PLATFORM_WINDOWS"
        }
        buildoptions "/utf-8"
    filter "configurations:Debug"
        defines "HZ_DEBUG"
        buildoptions "/MDd"
        symbols "On"
    filter "configurations:Release"
        defines "HZ_RELEASE"
        buildoptions "/MD"
        optimize "On"
    filter "configurations:Dist"
        defines "HZ_DIST"
        buildoptions "/MD"
        optimize "On"
