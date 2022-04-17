@ECHO OFF
REM this file is under creative commons 0
REM C:/VulkanSDK/1.3.204.1/Bin/glslc.exe -target-env=vulkan1.1 %1 -o %1.spv
C:/VulkanSDK/1.3.204.1/Bin/glslangValidator.exe -V %1 -o %1.spv
PAUSE

REM drag a file onto this batch to compile it
REM don't forget to add the output SPV to the Qt resource file, through the IDE