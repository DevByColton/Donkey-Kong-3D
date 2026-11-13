@echo off

cls

REM Trying this out, instead of having to use external powershell, this lets me build within sublime text
REM @call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Full Debug -Od -Zo
set OptimizationFlags=-Od -Zo -Oi -fp:fast

REM Release with debug SOME debug info from -Zo
REM set OptimizationFlags=-O2 -Zo -Oi -fp:fast

REM Release flags -O2
REM set OptimizationFlags=-O2 -Oi -fp:fast

set CPlusPlusVersion=-std:c++20
set WarningFlags=-W4 -wd4505 -wd4201 -wd4100 -wd4189 -wd4456
set CommonCompilerFlags=-WL -nologo -Gm- -GR- -EHa- %CPlusPlusVersion% %OptimizationFlags% %WarningFlags% -FC -Z7 -GS-
set CommonCompilerFlags=-DDEBUG=1 -DSIMD_AVX=1 -DSIMD_SSE=0 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib XAudio2.lib Xinput.lib d3d11.lib dxgi.lib

set DebugPath="C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Debug"

if not exist %DebugPath% mkdir %DebugPath%
pushd %DebugPath%

if exist *.rdi (del *.rdi > NUL 2 > NUL)
if exist *.pdb (del *.pdb > NUL 2 > NUL)
echo WAITING FOR PDB > lock.tmp
del lock.tmp

REM Compile shaders
set fxc="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe"
set ShaderDir=C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Data\

%fxc% /Fo "%ShaderDir%BaseVertexShader.cso" /T vs_5_0 "%ShaderDir%BaseVertexShader.hlsl"
%fxc% /Fo "%ShaderDir%BasePixelShader.cso" /T ps_5_0 "%ShaderDir%BasePixelShader.hlsl"
%fxc% /Fo "%ShaderDir%LightingPixelShader.cso" /T ps_5_0 "%ShaderDir%LightingPixelShader.hlsl"
%fxc% /Fo "%ShaderDir%BaseTexturePixelShader.cso" /T ps_5_0 "%ShaderDir%BaseTexturePixelShader.hlsl"
%fxc% /Fo "%ShaderDir%LightingTexturePixelShader.cso" /T ps_5_0 "%ShaderDir%LightingTexturePixelShader.hlsl"

cl %CommonCompilerFlags% "C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Code\Win32Main.cpp" -FmDonkeyKong3D.map -link -subsystem:windows -out:"Donkey Kong 3D.exe" %CommonLinkerFlags%
REM "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.51.36231\bin\Hostx64\x64\cl.exe" %CommonCompilerFlags% "C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Code\Win32Main.cpp" -FmDonkeyKong3D.map -link -subsystem:windows -out:"Donkey Kong 3D.exe" %CommonLinkerFlags%

popd