REM HUZZA FOR: https://dmerej.info/blog/post/cmake-visual-studio-and-the-command-line/
md .vs
cd .vs
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat"
cmake -DWIN32=on -G"NMake Makefiles" ..
nmake