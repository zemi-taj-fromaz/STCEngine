workspace "VulkanTest"
    architecture "x64"
    startproject "VulkanTest"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project("VulkanTest")
    location("VulkanTest")
    language("C++")
    cppdialect("C++17")
    kind("ConsoleApp")
    staticruntime("on")

    files{
        "%{prj.name}/src/*.cpp",
        "%{prj.name}/src/*.h"
    }

    includedirs
    {
        "libs/glm",
        "libs/glfw-3.3.8.bin.WIN64/include",
        "libs/VulkanSDK/1.3.261.0/Include",
        "src",
    }

    links {
        "vulkan-1.lib", -- add the names of the libraries you want to link against
        "glfw3_mt.lib"
        -- ... add more libraries as needed
    }
 
    libdirs {
        "C:/VulkanSDK/1.3.261.0/Lib",
        "C:/Users/pc/Downloads/glfw-3.3.8.bin.WIN64/lib-vc2022"
    }

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    filter "configurations.Debug"
    defines "HEAVEN_DEBUG"
    runtime "Debug"
    symbols "On"

    filter "configurations.Release"
        defines "HEAVEN_RELEASE"
        runtime "Release"
        symbols "On"

    filter "configurations.Dist"
        defines "HEAVEN_DIST"
        runtime "Release"
        symbols "On"