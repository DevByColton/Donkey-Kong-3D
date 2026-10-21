@echo off

cls

REM Release Optimized
set OptimizationFlags=-O2 -Oi -fp:fast

set CPlusPlusVersion=-std:c++20
set WarningFlags=-W4 -wd4505 -wd4201 -wd4100 -wd4189 -wd4456
set CommonCompilerFlags=-WL -nologo -Gm- -GR- -EHa- %CPlusPlusVersion% %OptimizationFlags% %WarningFlags% -FC -GS
set CommonCompilerFlags=-DNDEBUG -DSIMD_AVX=1 -DSIMD_SSE=0 %CommonCompilerFlags%
set CommonLinkerFlags=-incremental:no -opt:ref -opt:icf user32.lib XAudio2.lib Xinput.lib d3d11.lib dxgi.lib

set ReleasePath="C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Release"

if not exist %ReleasePath% mkdir %ReleasePath%

pushd %ReleasePath%

REM Compile shaders
set fxc="C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\fxc.exe"
set VertexShaderSrc="C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Data\CubeVertexShader.hlsl"
set VertexShaderOut=%ReleasePath%\CubeVertexShader.cso
set PixelShaderSrc="C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Data\CubePixelShader.hlsl"
set PixelShaderOut=%ReleasePath%\CubePixelShader.cso

%fxc% /Fo %VertexShaderOut% /T vs_5_0 %VertexShaderSrc%
%fxc% /Fo %PixelShaderOut% /T ps_5_0 %PixelShaderSrc%

REM Compile exe
cl %CommonCompilerFlags% "C:\Users\Colto\OneDrive\Desktop\Donkey Kong 3D\Code\Win32Main.cpp" -FmDonkeyKong3D.map -link -subsystem:windows -out:"Donkey Kong 3D.exe" %CommonLinkerFlags%

popd