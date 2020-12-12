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

::Set mypath:
cd %mypath%

@echo on

make PLATFORM=PARTNER

@echo off
::remove the ".o" extension from the wesslib temporarily:
ren "WESSLIB.o" "WESSLIB"

::remove_all:
	del *.o

::restore the ".o" extension of the wesslib:
ren "WESSLIB" "WESSLIB.o"
@echo on

pause
