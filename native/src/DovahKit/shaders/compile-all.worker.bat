@ECHO OFF
C:/VulkanSDK/1.3.204.1/Bin/glslangValidator.exe -V %1 -o %1.spv > _compile-all.bat.txt
FIND /C "ERROR:" _compile-all.bat.txt >NUL
IF %ERRORLEVEL% EQU 1 GOTO :EOF
TYPE _compile-all.bat.txt
PAUSE