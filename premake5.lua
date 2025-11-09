--[[  关于premake中的token（符记）： https://premake.github.io/docs/Tokens/  ]]

include "./vendor/premake/premake_customization/solution_items.lua"
include "premakeDependencies.lua"

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

---------------------------------------------------------------------------------------
--包含Nut/Nut/vendor/GLFW、Nut/Nut/vendor/Glad、Nut/Nut/vendor/imgui中的premake文件，将其作为依赖项，并合并到这里
group "Dependencies"
    include "vendor/premake"
    include "Hazel/vendor/GLFW"-- 检索这个目录下的lua文件，根据这些lua文件构建项目
    include "Hazel/vendor/Glad"
    include "Hazel/vendor/imgui"
    include "Hazel/vendor/yaml-cpp"
group "" 

include "Hazel"
include "Sandbox"
include "Hazelnut"