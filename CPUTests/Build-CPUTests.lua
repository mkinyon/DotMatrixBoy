project "CPUTests"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files
   { 
    "Source/**.h", 
    "Source/**.cpp",

    -- json
    "../ThirdParty/json/single_include/nlohmann/json.hpp"
   }

   includedirs
   {
      "Source",

      -- Include Core
	  "../Core/Source",

      -- json
      "../ThirdParty/json/single_include/nlohmann/"
   }

   links
   {
      "Core"
   }


   defines { "_CRT_SECURE_NO_WARNINGS" }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

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