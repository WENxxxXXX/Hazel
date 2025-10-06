--[[  关于premake中的token（符记）： https://premake.github.io/docs/Tokens/  ]]

workspace "Hazel"
    architecture "x64"
    startproject "Sandbox"          --[[启动项目]]
    configurations 
    { 
        "Debug",
        "Release",
        "Dist" 
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"--（方便输出和中间目录编写）

-- 包含相对根目录（解决方案目录）的目录
-- 定义全局变量，方便后面使用
IncludeDir = {}--创建一个名为IncludeDir的表
IncludeDir["GLFW"] = "Hazel/vendor/GLFW/include"--将表的"GLFW"键索引到此路径
IncludeDir["Glad"] = "Hazel/vendor/Glad/include"
IncludeDir["Imgui"] = "Hazel/vendor/imgui"
IncludeDir["glm"] = "Hazel/vendor/glm"

include "Hazel/vendor/GLFW"-- 检索这个目录下的lua文件，根据这些lua文件构建项目
include "Hazel/vendor/Glad"
include "Hazel/vendor/imgui"

project "Hazel"
    location "Hazel"        --项目文件的输出目录
    --kind "SharedLib"        --类型（动态库）
    kind "StaticLib"        --类型（静态库）
    language "C++"          --语言
    cppdialect "C++17"      --C++标准（编译时）
    staticruntime "on"

    targetdir  ("bin/" .. outputdir .. "/%{prj.name}")      --输出目录(.. XX ..中 ".."是字符串连接符)
    objdir  ("bin-int/" .. outputdir .. "/%{prj.name}")     --中间目录

    pchheader "hzpch.h"     --预编译头文件
    pchsource "Hazel/src/hzpch.cpp"--VS需要这个源文件

    files       --将源代码文件添加到特定的项目中
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

    includedirs --包含目录
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}", -- %{}使用全局变量
        "%{IncludeDir.Glad}",
        "%{IncludeDir.Imgui}",
        "%{IncludeDir.glm}"
    }

    links       --为项目(.dll)附加依赖项
    {
        "GLFW", --依赖GLFW项目
        "Glad",
        "ImGui",
        "opengl32.lib"
    }

    filter "system:windows"     --过滤器(如果系统是windows)
        systemversion "latest"  --windows SDK 版本
        defines                 --宏的声明
        {
            "HZ_PLATFORM_WINDOWS",
            "HZ_BUILD_DLL",
            --"GLFW_INCLUDE_NONE"
        }
        outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}" 
        postbuildcommands       --构建项目完成后执行的指令
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
