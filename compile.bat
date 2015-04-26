@echo off
for %%* in (.) do set CurrDirName=%%~n*
echo %CurrDirName%
set files=Terrain.cpp
set "command=g++ -pedantic -Wall -Werror -ggdb -g3 -std=c++11"
set "includes=-Iinclude -ISFML-2.1\include"
set "links=-LSFML-2.1\lib -lsfml-graphics-d -lsfml-window-d -lsfml-system-d -lopengl32 -lglu32 -lglut32"
@echo on
%command% %files% %includes% %links% -o %CurrDirName%
pause
%CurrDirName%.exe
pause