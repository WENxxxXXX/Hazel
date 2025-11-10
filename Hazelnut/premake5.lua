project "Hazelnut"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files                          
    {
        "src/**.h",
        "src/**.cpp"
    }
    
    defines
    {
        "YAML_CPP_STATIC_DEFINE"
    }

    includedirs                      
    {
        "%{wks.location}/Hazel/vendor/spdlog/include",
        "%{wks.location}/Hazel/src",
        "%{wks.location}/Hazel/vendor",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.Box2D}"
    }

    links
    {
        "Hazel"
    }

    filter "system:windows"        
        systemversion "latest"

        defines
        {
        
        }

        buildoptions "/utf-8"
        filter "configurations:Debug"
            defines "HZ_DEBUG"
            runtime "Debug"
            symbols "on"            

        filter "configurations:Release"
            defines "HZ_Release"
            runtime "Release"
            optimize "on"           

        filter "configurations:Dist"
            defines "HZ_DIST"
            runtime "Release"
            optimize "on"