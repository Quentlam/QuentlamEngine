project "Glad"
	kind "StaticLib"
	language "C"
	staticruntime "On"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/glad/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"
	}

	includedirs
	{
			"include"
	}

	filter "system:windows"
	systemversion "latest"

	filter "configurations:Debug"
		defines "QL_DEBUG"
		runtime "Debug"
		symbols "on"


	filter "configurations:Release"
		defines "QL_RELEASE"
		runtime "Release"
		optimize "on"
	