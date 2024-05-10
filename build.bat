@echo off

REM this will run a development build
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

SET includes=/I. /I%VULKAN_SDK%/Include /I"C:/Users/Matthew/glfw-3.3.9.bin.WIN64/glfw-3.3.9.bin.WIN64/include" /I"C:/Users/Matthew/tinyobjloader"
SET links=/link /LIBPATH:%VULKAN_SDK%/Lib /LIBPATH:"C:/Users/Matthew/glfw-3.3.9.bin.WIN64/glfw-3.3.9.bin.WIN64/lib-vc2022" vulkan-1.lib glfw3.lib User32.lib Gdi32.lib Shell32.lib
SET defines=/D DEBUG


echo "Building main..."

REM how to handle errors / crashes
cl /Zi /EHsc /MD %includes% %defines% *.cpp %links% /OUT:main.exe