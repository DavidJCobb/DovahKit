@ECHO OFF
REM this file is under creative commons 0
C:/VulkanSDK/1.2.189.2/Bin/glslc.exe %1 -o %1.spv
PAUSE

REM drag a file onto this batch to compile it
REM don't forget to add it to the Qt resource file, through the IDE