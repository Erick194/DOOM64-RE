@echo off

::Set Main Root:

set ROOT=c:\ultra

::Setup GCC:
set gccdir=%ROOT%\gcc
path %gccdir%\mipse\bin;%path%
set gccsw=-mips3 -mgp32 -mfp32 -funsigned-char -D_LANGUAGE_C -D_ULTRA64 -D__EXTENSIONS__
set n64align=on
set GCC_CELF=ON

::Setup LIB:
path %root%\usr\sbin;%path%

rem set Microsoft Visual C Ver 4.2 install directory
rem set MSVC=d:\msvc

rem set Borland C Ver 5.0 install directory
rem set BCC=d:\bc5

::Set mypath:
cd %mypath%

@echo on

make PLATFORM=PARTNER

@echo off
::remove_all:
	del *.o
@echo on

pause
