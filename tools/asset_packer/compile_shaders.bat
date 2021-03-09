@echo off

%VULKAN_SDK%\bin\glslangvalidator.exe -g bc7_encode.comp -o bc7_encode.h --vn bc7_encode_spirv --target-env vulkan1.2
%VULKAN_SDK%\bin\glslangvalidator.exe -g bc7_try_mode_456.comp -o bc7_try_mode_456.h --vn bc7_try_mode_456_spirv --target-env vulkan1.2