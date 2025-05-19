project "imgui"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
    staticruntime "on"

	targetdir ("bin/" .. outputs .. "/%{prj.name}")
	objdir ("bin-int/" .. outputs .. "/%{prj.name}")

	files
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",
		"imgui_impl_glfw.h",
		"imgui_impl_glfw.cpp",
		"imgui_impl_vulkan.h",
		"imgui_impl_vulkan.cpp",
	}

	includedirs
	{
		"%{prj.name}",
		"backends",
		"%{IncludeDir.glfw}/include",
		"%{IncludeDir.Vulkan}",
	}

	filter "system:windows"
		systemversion "latest"

	filter "system:linux"
		pic "On"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		
	filter "configurations:RelWithDebInfo"
		runtime "Release"
		symbols "on"
		optimize "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
