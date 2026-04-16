-- premake5.lua
workspace "ggpo"
   configurations { "Debug", "Release" }
   location "build"

   filter "system:Windows"
      defines { "_WINDOWS" }
      architecture "x86_64"

newoption {
   trigger = "steam",
   description = "Enable steam API"
}

project "ggpo"
   kind "StaticLib"
   language "C"
   cdialect "c11"
   warnings "High"
   -- fatalwarnings "All"
   -- targetdir "bin/%{cfg.buildcfg}"

   files { "src/include/**.h", "src/lib/ggpo/**.h", "src/lib/ggpo/**.c" }
   includedirs { "src/lib/ggpo", "src/include", "thirdparty/include" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   filter { "options:steam" }
      defines { "GGPO_STEAM" }
      removefiles { "src/lib/ggpo/network/connection.c" }

   filter { "not options:steam" }
      removefiles { "src/lib/ggpo/network/connection_steam.c" }
   filter { "options:steam", "system:Windows" }
      libdirs { "thirdparty/bin/win64" }
      links { "steam_api64" }
   filter { "options:steam", "system:linux" }
      libdirs { "thirdparty/bin/linux64" }
      links { "steam_api64" }

project "vectorwar"
   kind "WindowedApp"
   language "C"
   cdialect "c11"
   warnings "High"
   -- fatalwarnings "All"
   -- targetdir "bin/%{cfg.buildcfg}"

   files { "src/apps/vectorwar/**.h", "src/apps/vectorwar/**.c", "src/apps/vectorwar/**.rc" }
   includedirs { "src/apps/vectorwar", "src/include" }

   links { "ggpo" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   filter { "options:steam" }
      defines { "GGPO_STEAM" }
   filter { "options:steam", "system:Windows" }
      libdirs { "thirdparty/bin/win64" }
      links { "steam_api64" }
   filter { "options:steam", "system:linux" }
      libdirs { "thirdparty/bin/linux64" }
      links { "steam_api64" }
