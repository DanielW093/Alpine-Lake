@echo off

del /A:H *.suo
rmdir /S /Q Debug
rmdir /S /Q Release
rmdir /S /Q 3dgp\Debug
rmdir /S /Q 3dgp\Release
echo Done...
