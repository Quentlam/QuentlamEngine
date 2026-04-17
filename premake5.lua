workspace "QuentlamEngine"
	architecture "x64"
    characterset "Unicode" 
    buildoptions { "/utf-8" }
	startproject "QL-Editor"

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
IncludeDir["ImGuizmo"] = "QuentlamEngine/vendor/ImGuizmo"
IncludeDir["glm"] = "QuentlamEngine/vendor/glm"
IncludeDir["entt"] = "QuentlamEngine/vendor/entt/src"
IncludeDir["stb_image"] = "QuentlamEngine/vendor/stb_image"
IncludeDir["assimp"] = "QuentlamEngine/vendor/assimp/include"
IncludeDir["assimp_build"] = "QuentlamEngine/vendor/assimp/build/include"
IncludeDir["Box2D"] = "QuentlamEngine/vendor/Box2D/include"
IncludeDir["JoltPhysics"] = "QuentlamEngine/vendor/JoltPhysics"

include "QuentlamEngine/vendor/GLFW"
include "QuentlamEngine/vendor/Glad"
include "QuentlamEngine/vendor/ImGui"

project "Box2D"
	location "QuentlamEngine/vendor/Box2D"
	kind "Staticlib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir("bin/" ..outputdir.. "/%{prj.name}")
	objdir("bin-int/" ..outputdir.. "/%{prj.name}")

	files
	{
		"QuentlamEngine/vendor/Box2D/src/**.cpp",
		"QuentlamEngine/vendor/Box2D/include/**.h"
	}

	includedirs
	{
		"QuentlamEngine/vendor/Box2D/include",
		"QuentlamEngine/vendor/Box2D/src"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		
	filter "configurations:Release"
		runtime "Release"
		
	filter "configurations:Dist"
		runtime "Release"

project "JoltPhysics"
	location "QuentlamEngine/vendor/JoltPhysics"
	kind "Staticlib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir("bin/" ..outputdir.. "/%{prj.name}")
	objdir("bin-int/" ..outputdir.. "/%{prj.name}")

	files
	{
		"QuentlamEngine/vendor/JoltPhysics/Jolt/**.cpp",
		"QuentlamEngine/vendor/JoltPhysics/Jolt/**.h",
		"QuentlamEngine/vendor/JoltPhysics/Jolt/**.inl"
	}

	includedirs
	{
		"QuentlamEngine/vendor/JoltPhysics"
	}
	
	defines
	{
		"JPH_PROFILE_ENABLED",
		"JPH_DEBUG_RENDERER"
	}

	filter "system:windows"
		systemversion "latest"
		
	filter "configurations:Debug"
		defines { "JPH_PROFILE_ENABLED", "JPH_DEBUG_RENDERER" }
		runtime "Debug"
		
	filter "configurations:Release"
		defines { "JPH_PROFILE_ENABLED", "JPH_DEBUG_RENDERER" }
		runtime "Release"
		
	filter "configurations:Dist"
		defines {}
		runtime "Release"

	

project "QuentlamEngine"
	location "QuentlamEngine"
	kind "Staticlib"
	language "C++"
	cppdialect "C++20"
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
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.assimp_build}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.JoltPhysics}"
	}


	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"Box2D",
		"JoltPhysics",
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
		links { "QuentlamEngine/vendor/assimp/build/lib/Debug/assimp-vc143-mtd.lib", "QuentlamEngine/vendor/assimp/build/contrib/zlib/Debug/zlibstaticd.lib" }

	
	filter "configurations:Release"
		defines "QL_RELEASE"
		runtime "Release"
		optimize "on"
		links { "QuentlamEngine/vendor/assimp/build/lib/Release/assimp-vc143-mt.lib", "QuentlamEngine/vendor/assimp/build/contrib/zlib/Release/zlibstatic.lib" }

		
	filter "configurations:Dist"
		defines "QL_DIST"
		runtime "Release"
		optimize "on"
		links { "QuentlamEngine/vendor/assimp/build/lib/Release/assimp-vc143-mt.lib", "QuentlamEngine/vendor/assimp/build/contrib/zlib/Release/zlibstatic.lib" }


project "Sandbox"
	location "Sandbox"
	kind "ConsoleAPP"
	language "C++"
	cppdialect "C++20"
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
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.assimp_build}"
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



project "QL-Editor"
	location "QL-Editor"
	kind "ConsoleAPP"
	language "C++"
	cppdialect "C++20"
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
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.assimp_build}"
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

