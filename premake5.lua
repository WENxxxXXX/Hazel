--[[  关于premake中的token（符记）： https://premake.github.io/docs/Tokens/  ]]

workspace "Hazel"
    architecture "x64"
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

include "Hazel/vendor/GLFW"-- 检索这个目录下的lua文件，根据这些lua文件构建项目

project "Hazel"
    location "Hazel"        --项目文件的输出目录
    kind "SharedLib"        --类型（动态库）
    language "C++"          --语言

    targetdir  ("bin/" .. outputdir .. "/%{prj.name}")      --输出目录(.. XX ..中 ".."是字符串连接符)
    objdir  ("bin-int/" .. outputdir .. "/%{prj.name}")     --中间目录

    pchheader "hzpch.h"     --预编译头文件
    pchsource "Hazel/src/hzpch.cpp"--VS需要这个源文件

    files       --将源代码文件添加到特定的项目中
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs --包含目录
    {
        "%{prj.name}/src",
        "%{prj.name}/vendor/spdlog/include",
        "%{IncludeDir.GLFW}"-- %{}使用全局变量
    }

    links       --为项目(.dll)附加依赖项
    {
        "GLFW", --依赖GLFW项目
        "opengl32.lib"
    }

    filter "system:windows"     --过滤器(如果系统是windows)
        cppdialect "C++17"      --C++标准（编译时）
        staticruntime "On"      --是否静态链接运行时库（dll属性的文件需要打开）
        systemversion "latest"  --windows SDK 版本
        defines                 --宏的声明
        {
            "HZ_PLATFORM_WINDOWS",
            "HZ_BUILD_DLL"
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
