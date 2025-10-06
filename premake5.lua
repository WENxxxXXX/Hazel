--[[  ����premake�е�token�����ǣ��� https://premake.github.io/docs/Tokens/  ]]

workspace "Hazel"
    architecture "x64"
    startproject "Sandbox"          --[[������Ŀ]]
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
IncludeDir["Glad"] = "Hazel/vendor/Glad/include"
IncludeDir["Imgui"] = "Hazel/vendor/imgui"
IncludeDir["glm"] = "Hazel/vendor/glm"

include "Hazel/vendor/GLFW"-- �������Ŀ¼�µ�lua�ļ���������Щlua�ļ�������Ŀ
include "Hazel/vendor/Glad"
include "Hazel/vendor/imgui"

project "Hazel"
    location "Hazel"        --��Ŀ�ļ������Ŀ¼
    --kind "SharedLib"        --���ͣ���̬�⣩
    kind "StaticLib"        --���ͣ���̬�⣩
    language "C++"          --����
    cppdialect "C++17"      --C++��׼������ʱ��
    staticruntime "on"

    targetdir  ("bin/" .. outputdir .. "/%{prj.name}")      --���Ŀ¼(.. XX ..�� ".."���ַ������ӷ�)
    objdir  ("bin-int/" .. outputdir .. "/%{prj.name}")     --�м�Ŀ¼

    pchheader "hzpch.h"     --Ԥ����ͷ�ļ�
    pchsource "Hazel/src/hzpch.cpp"--VS��Ҫ���Դ�ļ�

    files       --��Դ�����ļ���ӵ��ض�����Ŀ��
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs --����Ŀ¼
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}", -- %{}ʹ��ȫ�ֱ���
        "%{IncludeDir.Glad}",
        "%{IncludeDir.Imgui}",
        "%{IncludeDir.glm}"
    }

    links       --Ϊ��Ŀ(.dll)����������
    {
        "GLFW", --����GLFW��Ŀ
        "Glad",
        "ImGui",
        "opengl32.lib"
    }

    filter "system:windows"     --������(���ϵͳ��windows)
        systemversion "latest"  --windows SDK �汾
        defines                 --�������
        {
            "HZ_PLATFORM_WINDOWS",
            "HZ_BUILD_DLL",
            --"GLFW_INCLUDE_NONE"
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
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "HZ_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "HZ_DIST"
        runtime "Release"
        optimize "on"


project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

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
        "Hazel/src",
        "%{IncludeDir.glm}",
        "Hazel/vendor"
    }

    links
    {
        "Hazel"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "HZ_PLATFORM_WINDOWS"
        }

        buildoptions "/utf-8"
    filter "configurations:Debug"
        defines "HZ_DEBUG"
        runtime "Debug"
        symbols "on"
    filter "configurations:Release"
        defines "HZ_RELEASE"
        runtime "Release"
        optimize "on"
    filter "configurations:Dist"
        defines "HZ_DIST"
        runtime "Release"
        optimize "on"
