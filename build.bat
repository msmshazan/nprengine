@echo off
set PATH=%PATH%;%cd%\tools\;
set CompilerFlags= /FC /nologo /Z7 /EHsc /wd4003 /wd4551 /MT /Fonpr
set LinkerFlags=-subsystem:windows
set bits=x64
set LibraryLocation=..\deps\libs\%bits%\
set LinkLibraries= advapi32.lib gdi32.lib winmm.lib user32.lib Shcore.lib kernel32.lib bgfx-shared-libRelease.lib meshoptimizer.lib
mkdir build > NUL 2> NUL

IF NOT DEFINED vcvars_called (
      pushd %cd%
	  set vcvars_called=1
	  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" %bits% > NUL 2>NUL 
	  popd )

cd build
del *.pdb > NUL 2> NUL
REM shaderc -f ..\code\vs_texquad.sc -o vs_texquad.bin -i ..\deps\include\bgfx --type v --platform windows --debug --disasm --profile vs_5_0 -O 3  --varyingdef ..\code\varying.def.sc
REM shaderc -f ..\code\fs_texquad.sc -o fs_texquad.bin -i ..\deps\include\bgfx --type f --platform windows --debug --disasm --profile ps_5_0 -O 3 --varyingdef ..\code\varying.def.sc
shaderc -f ..\code\vs_imgui.sc -o vs_imgui.bin -i ..\deps\include\bgfx --type v --platform windows --debug --disasm --profile vs_5_0 -O 3  --varyingdef ..\code\imgui_varying.def.sc
shaderc -f ..\code\fs_imgui.sc -o fs_imgui.bin -i ..\deps\include\bgfx --type f --platform windows --debug --disasm --profile ps_5_0 -O 3 --varyingdef ..\code\imgui_varying.def.sc
shaderc -f ..\code\vs_mesh.sc -o vs_mesh.bin -i ..\deps\include\bgfx --type v --platform windows --debug --disasm --profile vs_5_0 -O 3  --varyingdef ..\code\mesh_varying.def.sc
shaderc -f ..\code\fs_mesh.sc -o fs_mesh.bin -i ..\deps\include\bgfx --type f --platform windows --debug --disasm --profile ps_5_0 -O 3 --varyingdef ..\code\mesh_varying.def.sc
REM ..\tools\shaderc.exe -f ..\code\fs_textilemap.sc -o fs_textilemap.bin -i ..\deps\include\bgfx --type f --platform windows --debug --disasm --profile ps_5_0 -O 3 --varyingdef ..\code\varying.def.sc
cl %CompilerFlags% ..\code\main.cpp  /I..\deps\include /link -incremental:no /LIBPATH:%LibraryLocation%  %LinkLibraries% %LinkerFlags%
REM "C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\gflags.exe" /p /enable npr.exe /full
xcopy %LibraryLocation%*.dll . /Q /Y
REM TexturePacker.exe ATLAS ..\assets\images -j -u -v -x -b -p 
REM TexturePacker.exe Test ..\assets\test -j -u -v -x -b -p -r
REM geometryc -f ..\assets\cube.glb -o cube.bin --packnormal 1
geometryc -f ..\assets\cube.glb -o cube.bin --packnormal 1 > NUL 2> NUL
cd ..
