-- premake5.lua
workspace "DotMatrixBoy"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "App"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

-- copy the SDL2.dll file
p = path.getabsolute("path", "relativeTo")
postbuildcommands {
   "xcopy %{wks.location}ThirdParty\\SDL2\\lib\\x64\\SDL2.dll %{wks.location}Binaries\\%{cfg.system}-%{cfg.architecture}\\%{cfg.buildcfg}\\App\\ /y /d"
 }


include "Core/Build-Core.lua"
include "App/Build-App.lua"
include "CPUTests/Build-CPUTests.lua"
