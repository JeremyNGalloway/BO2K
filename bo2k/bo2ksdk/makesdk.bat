@echo off
echo Copying SDK files...
deltree /y include
mkdir include
copy ..\include\auth.h include
copy ..\include\bocomreg.h include
copy ..\include\comm_native.h include
copy ..\include\config.h include
copy ..\include\encryption.h include
copy ..\include\iohandler.h include
copy ..\include\plugins.h include
copy ..\include\strhandle.h include
copy ..\include\commnet.h include
deltree /y src
mkdir src
copy ..\src\misc\config\config.cpp src
copy ..\src\libc\fake_libc.c src
copy ..\src\libc\newdelete.cpp src
copy ..\src\libc\strhandle.cpp src
copy ..\src\commands\commnet.cpp src
deltree /y sample\reference
mkdir sample\reference
copy ..\src\io\io_simpletcp.cpp sample\reference
copy ..\src\encryption\xorencrypt.cpp sample\reference
copy ..\src\auth\nullauth.cpp sample\reference
echo Files copied.
start .

