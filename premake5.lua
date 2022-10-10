-- premake5.lua
workspace "WobBT"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "WobBT"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "WobBT"