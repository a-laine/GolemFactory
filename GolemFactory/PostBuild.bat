@echo off
Rem This script copy all dll files from directories into target directories

if NOT %1%==.exe GOTO end

echo Post build script
Rem echo type : %1
Rem echo target folder : %2
Rem echo tools directory : %3
Rem echo configuration : %4
Rem echo plateform : %5

echo copy glfw3.dll
robocopy "%3\1-GLFW\lib %5" %2 glfw3.dll /w:2 /log:%2log.txt >nul

echo copy glew32.dll
robocopy "%3\2-GLEW\lib %5" %2 glew32.dll /w:2 /log+:%2log.txt >nul

echo copy assimp-vc140-mt.dll
robocopy "%3\4-ASSIMP\lib %5 %4" %2 assimp-vc140-mt.dll /w:2 /log+:%2log.txt >nul

Rem this error is just for devellopement of this script (to avoid recompiling the project)
Rem EXIT /B 1

:end
EXIT /B 0

