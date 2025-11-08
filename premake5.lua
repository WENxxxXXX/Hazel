--[[  关于premake中的token（符记）： https://premake.github.io/docs/Tokens/  ]]

workspace "Hazel"
    architecture "x86_64"
    startproject "Hazelnut"          --[[启动项目]]
    configurations 
    { 
        "Debug",
        "Release",
        "Dist" 
    }

    flags                           --设置编译器选项
	{
		"MultiProcessorCompile"     --多处理器并行编译
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"--（方便输出和中间目录编写）

-- 包含相对根目录（解决方案目录）的目录
-- 定义全局变量，方便后面使用
IncludeDir = {}--创建一个名为IncludeDir的表
IncludeDir["GLFW"] = "%{wks.location}/Hazel/vendor/GLFW/include"--将表的"GLFW"键索引到此路径
IncludeDir["Glad"] = "%{wks.location}/Hazel/vendor/Glad/include"
IncludeDir["Imgui"] = "%{wks.location}/Hazel/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Hazel/vendor/glm"
IncludeDir["stb_image"] = "%{wks.location}/Hazel/vendor/stb_image"
IncludeDir["entt"] = "%{wks.location}/Hazel/vendor/Entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/Hazel/vendor/yaml-cpp/include"

---------------------------------------------------------------------------------------
--包含Nut/Nut/vendor/GLFW、Nut/Nut/vendor/Glad、Nut/Nut/vendor/imgui中的premake文件，将其作为依赖项，并合并到这里
group "Dependencies"
    include "Hazel/vendor/GLFW"-- 检索这个目录下的lua文件，根据这些lua文件构建项目
    include "Hazel/vendor/Glad"
    include "Hazel/vendor/imgui"
    include "Hazel/vendor/yaml-cpp"
group "" 

include "Hazel"
include "Sandbox"
include "Hazelnut"