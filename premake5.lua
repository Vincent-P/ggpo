-- premake5.lua
workspace "ggpo"
   configurations { "Debug", "Release" }
   location "build"

project "ggpo"
   kind "StaticLib"
   language "C"
   cdialect "c11"
   warnings "High"
   -- fatalwarnings "All"
   -- targetdir "bin/%{cfg.buildcfg}"

   files { "src/include/**.h", "src/lib/ggpo/**.h", "src/lib/ggpo/**.c" }
   includedirs { "src/lib/ggpo", "src/include" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"
