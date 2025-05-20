
------------ path ------------
vendor_path = {}
vendor_path["glew"]				= "vendor/glew"
vendor_path["glfw"]          	= "vendor/glfw"
vendor_path["glm"]           	= "vendor/glm"
vendor_path["ImGui"]         	= "vendor/imgui"
vendor_path["tinyobjloader"] 	= "vendor/tinyobjloader"
vendor_path["stb_image"]     	= "vendor/stb_image"
vendor_path["ImGuizmo"]         = "vendor/ImGuizmo"

------------ include ------------ 
IncludeDir = {}
IncludeDir["glfw"]				= "%{wks.location}/%{vendor_path.glfw}"
IncludeDir["glfw"]              = "%{wks.location}/%{vendor_path.glfw}"
IncludeDir["glm"]               = "%{wks.location}/%{vendor_path.glm}"
IncludeDir["ImGui"]             = "%{wks.location}/%{vendor_path.ImGui}"
IncludeDir["tinyobjloader"]     = "%{wks.location}/%{vendor_path.tinyobjloader}"
IncludeDir["stb_image"]         = "%{wks.location}/%{vendor_path.stb_image}"
IncludeDir["ImGuizmo"]          = "%{wks.location}/%{vendor_path.ImGuizmo}"




workspace "gluttony"
	platforms "x64"
	startproject "gluttony"

	configurations
	{
		"Debug",
		"RelWithDebInfo",
		"Release",
	}

	flags
	{
		"MultiProcessorCompile"
	}

	outputs  = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	local project_path = os.getcwd()
	defines
	{
		"ENGINE_INSTALL_DIR=\"" .. project_path .. "/bin/" .. outputs .. "\"",
		"OUTPUTS=\"" .. outputs .. "\"",
	}

	if os.target() == "linux" then
		print("---------- target platform is linux => manually compile GLFW ----------")
		os.execute("cmake -S ./vendor/glfw -B ./vendor/glfw/build")			-- manuel compilation
		os.execute("cmake --build ./vendor/glfw/build")						-- manuel compilation
		print("---------- Done compiling GLFW ----------")
	end

group "dependencies"
	include "vendor/imgui"
	if os.target() == "windows" then
		include "vendor/glfw"
	end
group ""




project "gluttony"
	location "%{wks.location}"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputs  .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputs  .. "/%{prj.name}")
	
	pchheader "util/pch.h"
	pchsource "src/util/pch.cpp"

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
	}

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.embed",

		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
	}

	includedirs
	{
		"src",
		"assets",
		"vendor",
        
        "%{IncludeDir.glew}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.glfw}/include",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGui}/backends/",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
	}
	
	links
	{
		"ImGui",
	}



	libdirs 
	{
		"vendor/imgui/bin/" .. outputs .. "/ImGui",
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }
	
	filter "system:linux"
		systemversion "latest"
		defines "PLATFORM_LINUX"

		includedirs
		{
			"/usr/include/x86_64-linux-gnu/qt5", 				-- Base Qt include path
			"/usr/include/x86_64-linux-gnu/qt5/QtCore",
			"/usr/include/x86_64-linux-gnu/qt5/QtWidgets",
			"/usr/include/x86_64-linux-gnu/qt5/QtGui",
		}
	
		libdirs
		{
			"%{wks.location}/vendor/glfw/build/src",
			"/usr/lib/x86_64-linux-gnu",
			"/usr/lib/x86_64-linux-gnu/qt5",
		}
	
		links
		{
			"GLEW",
        	"GL",
			"glfw3",
			"Qt5Core",
			"Qt5Widgets",
			"Qt5Gui",
		}

		buildoptions
		{
			"-msse4.1",										  	-- include the SSE4.1 flag for Linux builds
			"-fPIC",

			-- compiler options
			"-Wall",
        	"-Wno-dangling-else",
		}

		postbuildcommands
		{

			'{COPYDIR} "%{wks.location}/shaders" "%{wks.location}/bin/' .. outputs .. '"',
			'{COPYDIR} "%{wks.location}/assets" "%{wks.location}/bin/' .. outputs .. '"',
		}
		
	filter "configurations:Debug"
		defines "DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:RelWithDebInfo"
		defines "RELEASE_WITH_DEBUG_INFO"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:Release"
		defines "RELEASE"
		runtime "Release"
		symbols "off"
		optimize "on"
