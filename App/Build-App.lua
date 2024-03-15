project "App"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { 
    "Source/**.h", 
    "Source/**.cpp", 
    
    -- imgui
    "../ThirdParty/imgui/imconfig.h",
    "../ThirdParty/imgui/imgui.h",
    "../ThirdParty/imgui/imgui.cpp",
    "../ThirdParty/imgui/imgui_draw.cpp",
    "../ThirdParty/imgui/imgui_internal.h",
    "../ThirdParty/imgui/imgui_tables.cpp",
    "../ThirdParty/imgui/imgui_widgets.cpp",
    "../ThirdParty/imgui/imstb_rectpack.h",
    "../ThirdParty/imgui/imstb_textedit.h",
    "../ThirdParty/imgui/imstb_truetype.h",
    "../ThirdParty/imgui/imgui_demo.cpp",

    -- imgui backends
    "../ThirdParty/imgui/backends/imgui_impl_sdl2.cpp",
    "../ThirdParty/imgui/backends/imgui_impl_sdl2.h",
    "../ThirdParty/imgui/backends/imgui_impl_sdlrenderer2.cpp",
    "../ThirdParty/imgui/backends/imgui_impl_sdlrenderer2.h",

    -- imgui file browser
    "../ThirdParty/imgui-filebrowser/imfilebrowser.h",
    }
   

    defines { "_CRT_SECURE_NO_WARNINGS" }

   includedirs
   {
      "Source",

	  -- Include Core
	  "../Core/Source",

      "../ThirdParty/imgui",
      "../ThirdParty/imgui/backends",
      "../ThirdParty/imgui-filebrowser",
      "../ThirdParty/OneLoneCoder",
      "../ThirdParty/SDL2/include"
   }

   libdirs {
      "../ThirdParty/SDL2/lib/x64/"
   }

   links
   {
      "Core",
      "SDL2",
      "SDL2main"
   }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"