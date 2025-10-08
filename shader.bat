@echo off
setlocal
set VULKANVERSION="1.4.313.0"
set VulkanSDK="C:\VulkanSDK\%VULKANVERSION%\Bin\glslangValidator.exe"
%VulkanSDK% -V shaders/triangle.vert -o shaders/triangle.vert.spv
%VulkanSDK% -V shaders/triangle.frag -o shaders/triangle.frag.spv

endlocal
