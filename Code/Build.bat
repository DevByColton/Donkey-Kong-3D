@echo off

cls

REM Full Debug -Od -Zo
set OptimizationFlags=-Od -Zo -Oi -fp:fast

REM Release with debug SOME debug info from -Zo
REM set OptimizationFlags=-O2 -Zo -Oi -fp:fast

REM Release flags -O2
REM set OptimizationFlags=-O2 -Oi -fp:fast

set CPlusPlusVersion=-std:c++20
set WarningFlags=-W4 -wd4505 -wd4201 -wd4100 -wd4189 -wd4456
set CommonCompilerFlags=-WL -nologo -MTd -Gm- -GR- -EHa- %CPlusPlusVersion% %OptimizationFlags% %WarningFlags% -FC -Z7 -GS-
set CommonCompilerFlags=-DDEBUG=1 -DSIMD_AVX=1 -DSIMD_SSE=0 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib Winmm.lib XAudio2.lib Xinput.lib d3d11.lib dxgi.lib

set DebugPath="C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Build"

if not exist %DebugPath% mkdir %DebugPath%
pushd %DebugPath%

if exist *.rdi (del *.rdi > NUL 2 > NUL)
if exist *.pdb (del *.pdb > NUL 2 > NUL)
echo WAITING FOR PDB > lock.tmp
del lock.tmp

cl %CommonCompilerFlags% "C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Code\Win32Main.cpp" -FmDonkeyKong3D.map -link -subsystem:windows %CommonLinkerFlags%

popd