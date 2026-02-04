workspace "QuentlamEngine"
	architecture "x64"
    characterset "Unicode" 
    buildoptions { "/utf-8" }
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


IncludeDir = {}
IncludeDir["GLFW"] = "QuentlamEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "QuentlamEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "QuentlamEngine/vendor/imgui"
IncludeDir["glm"] = "QuentlamEngine/vendor/glm"
IncludeDir["stb_image"] = "QuentlamEngine/vendor/stb_image"

include "QuentlamEngine/vendor/GLFW"
include "QuentlamEngine/vendor/Glad"
include "QuentlamEngine/vendor/ImGui"

	

project "QuentlamEngine"
	location "QuentlamEngine"
	kind "Staticlib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" ..outputdir.. "/%{prj.name}")
	objdir("bin-int/" ..outputdir.. "/%{prj.name}")

	pchheader "qlpch.h"
	pchsource "QuentlamEngine/src/qlpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp"

	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"

	}


	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

		defines
		{
			"QL_PLATFORM_WINDOWS",
			"QL_BUILD_DLL",
			"_WINDLL",
			"GLFW_INCLUDE_NONE"
		}


	filter "configurations:Debug"
		defines "QL_DEBUG"
		runtime "Debug"
		symbols "on"

	
	filter "configurations:Release"
		defines "QL_RELEASE"
		runtime "Release"
		optimize "on"

		
	filter "configurations:Dist"
		defines "QL_DIST"
		runtime "Release"
		optimize "on"


project "Sandbox"
	location "Sandbox"
	kind "ConsoleAPP"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir("bin/" ..outputdir.. "/%{prj.name}")
	objdir("bin-int/" ..outputdir.. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",

	}

	includedirs
	{
		"QuentlamEngine/vendor/spdlog/include",
		"QuentlamEngine/src",
		"QuentlamEngine/vendor",
		"%{IncludeDir.glm}"
	}

	links
	{
		"QuentlamEngine"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

	defines
	{
		"QL_PLATFORM_WINDOWS"
	}


	filter "configurations:Debug"
		defines "QL_DEBUG"
		runtime "Debug"
		symbols "on"

	
	filter "configurations:Release"
		defines "QL_RELEASE"
		runtime "Release"
		optimize "on"

		
	filter "configurations:Dist"
		defines "QL_DIST"
		runtime "Release"
		optimize "on"

