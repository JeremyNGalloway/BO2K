# Microsoft Developer Studio Project File - Name="bo2k" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=bo2k - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bo2k.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bo2k.mak" CFG="bo2k - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bo2k - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "bo2k - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/bo2k", ZJBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bo2k - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O1 /I "\projects\bo2k\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__BO2KSERVER__" /FR /YX /Gs16384 /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib vfw32.lib winmm.lib /nologo /base:"0x03140000" /version:1.0 /entry:"WinMain@16" /subsystem:windows /pdb:none /map /machine:I386 /nodefaultlib

!ELSEIF  "$(CFG)" == "bo2k - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Zi /Od /I "\projects\bo2k\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__BO2KSERVER__" /FR /YX /Gs65536 /FD /I /GD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib vfw32.lib winmm.lib /nologo /base:"0x03140000" /version:1.0 /entry:"WinMain@16" /subsystem:windows /incremental:no /map /debug /machine:I386 /nodefaultlib /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "bo2k - Win32 Release"
# Name "bo2k - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "lzh"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\lzh\clzhcompress.cpp
# End Source File
# End Group
# Begin Group "commands"

# PROP Default_Filter ""
# Begin Group "simple"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\simple\cmd_simple.cpp

!IF  "$(CFG)" == "bo2k - Win32 Release"

!ELSEIF  "$(CFG)" == "bo2k - Win32 Debug"

# ADD CPP /vd1 /GR- /GX-

!ENDIF 

# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\system\cmd_system.cpp
# End Source File
# Begin Source File

SOURCE=.\src\commands\system\dumppw.cpp
# End Source File
# End Group
# Begin Group "keylogging"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\keylogging\cmd_keylogging.cpp
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\gui\cmd_gui.cpp
# End Source File
# End Group
# Begin Group "msnet"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\msnet\cmd_msnet.cpp
# End Source File
# End Group
# Begin Group "process"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\process\cmd_process.cpp
# End Source File
# End Group
# Begin Group "registry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\registry\cmd_registry.cpp
# End Source File
# End Group
# Begin Group "multimedia"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\multimedia\cmd_multimedia.cpp

!IF  "$(CFG)" == "bo2k - Win32 Release"

# ADD CPP /Gd

!ELSEIF  "$(CFG)" == "bo2k - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "plugin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\plugin\cmd_plugin.cpp
# End Source File
# End Group
# Begin Group "file"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\file\cmd_file.cpp
# End Source File
# End Group
# Begin Group "compress"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\compress\cmd_compress.cpp
# End Source File
# End Group
# Begin Group "resolver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\resolver\cmd_resolver.cpp
# End Source File
# End Group
# Begin Group "serverctrl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\serverctrl\cmd_serverctrl.cpp
# End Source File
# End Group
# Begin Group "tcpip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\commands\tcpip\cmd_tcpip.cpp
# End Source File
# Begin Source File

SOURCE=.\src\commands\tcpip\cmd_tcpip_app.cpp
# End Source File
# Begin Source File

SOURCE=.\src\commands\tcpip\cmd_tcpip_filerecv.cpp
# End Source File
# Begin Source File

SOURCE=.\src\commands\tcpip\cmd_tcpip_http.cpp
# End Source File
# Begin Source File

SOURCE=.\src\commands\tcpip\cmd_tcpip_redir.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\commands\bocomreg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\commands\comm_native.cpp
# End Source File
# Begin Source File

SOURCE=.\src\commands\commnet.cpp
# End Source File
# End Group
# Begin Group "io"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\io\io_simpletcp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\io\io_simpleudp.cpp
# End Source File
# Begin Source File

SOURCE=.\src\io\iohandler.cpp
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Group "proclist"

# PROP Default_Filter ""
# Begin Group "nt-pviewer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\src\misc\proclist\nt-pviewer\cntrdata.cpp"
# End Source File
# Begin Source File

SOURCE=".\src\misc\proclist\nt-pviewer\instdata.cpp"
# End Source File
# Begin Source File

SOURCE=".\src\misc\proclist\nt-pviewer\nt_pviewer.cpp"
# End Source File
# Begin Source File

SOURCE=".\src\misc\proclist\nt-pviewer\objdata.cpp"
# End Source File
# Begin Source File

SOURCE=".\src\misc\proclist\nt-pviewer\perfdata.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\misc\proclist\pviewer.cpp
# End Source File
# End Group
# Begin Group "osversion"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\misc\osversion\osversion.cpp
# End Source File
# End Group
# Begin Group "images"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\misc\images\cdrom.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\cdromgif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\computer.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\computergif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\domain.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\domaingif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\drive.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\drivegif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\entirenetwork.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\entirenetworkgif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\exe.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\exegif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\file.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\filegif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\folder.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\foldergif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\html.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\htmlgif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\image.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\imagegif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\network.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\networkgif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\printer.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\printergif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\remote.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\remotegif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\server.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\servergif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\text.gif
# End Source File
# Begin Source File

SOURCE=.\src\misc\images\textgif.cpp
# End Source File
# End Group
# Begin Group "permissions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\misc\permissions\permissions.cpp
# End Source File
# End Group
# Begin Group "config"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\misc\config\config.cpp
# End Source File
# End Group
# Begin Group "process_hop"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\misc\process_hop\process_hop.cpp
# End Source File
# End Group
# End Group
# Begin Group "dynamic"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dynamic\functions.cpp
# End Source File
# End Group
# Begin Group "libc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\libc\fake_libc.c
# End Source File
# Begin Source File

SOURCE=.\src\libc\newdelete.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libc\strhandle.cpp
# End Source File
# End Group
# Begin Group "plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\plugins\plugins.cpp
# End Source File
# End Group
# Begin Group "encryption"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\encryption\deshash.cpp
# End Source File
# Begin Source File

SOURCE=.\src\encryption\encryption.cpp
# End Source File
# Begin Source File

SOURCE=.\src\encryption\xorencrypt.cpp
# End Source File
# End Group
# Begin Group "auth"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\auth\auth.cpp
# End Source File
# Begin Source File

SOURCE=.\src\auth\nullauth.cpp
# End Source File
# End Group
# Begin Group "dll"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\dll\dll_load.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\commandloop.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "cmd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\cmd\cmd_compress.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_file.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_gui.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_keylogging.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_msnet.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_multimedia.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_plugin.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_process.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_registry.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_resolver.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_serverctrl.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_simple.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_system.h
# End Source File
# Begin Source File

SOURCE=.\include\cmd\cmd_tcpip.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\auth.h
# End Source File
# Begin Source File

SOURCE=.\include\bo_debug.h
# End Source File
# Begin Source File

SOURCE=.\include\bocomreg.h
# End Source File
# Begin Source File

SOURCE=.\include\comm_native.h
# End Source File
# Begin Source File

SOURCE=.\include\commandloop.h
# End Source File
# Begin Source File

SOURCE=.\include\commnet.h
# End Source File
# Begin Source File

SOURCE=.\include\config.h
# End Source File
# Begin Source File

SOURCE=.\include\deshash.h
# End Source File
# Begin Source File

SOURCE=.\include\dll_load.h
# End Source File
# Begin Source File

SOURCE=.\include\dumppw.h
# End Source File
# Begin Source File

SOURCE=.\include\encryption.h
# End Source File
# Begin Source File

SOURCE=.\include\functions.h
# End Source File
# Begin Source File

SOURCE=.\include\io_simpletcp.h
# End Source File
# Begin Source File

SOURCE=.\include\io_simpleudp.h
# End Source File
# Begin Source File

SOURCE=.\include\iohandler.h
# End Source File
# Begin Source File

SOURCE=.\include\Lzh.h
# End Source File
# Begin Source File

SOURCE=.\include\lzhcompress.h
# End Source File
# Begin Source File

SOURCE=.\include\main.h
# End Source File
# Begin Source File

SOURCE=.\include\nt_pviewer.h
# End Source File
# Begin Source File

SOURCE=.\include\nullauth.h
# End Source File
# Begin Source File

SOURCE=.\include\osversion.h
# End Source File
# Begin Source File

SOURCE=.\include\perfdata.h
# End Source File
# Begin Source File

SOURCE=.\include\permissions.h
# End Source File
# Begin Source File

SOURCE=.\include\plugins.h
# End Source File
# Begin Source File

SOURCE=.\include\process_hop.h
# End Source File
# Begin Source File

SOURCE=.\include\pviewdat.h
# End Source File
# Begin Source File

SOURCE=.\include\pviewer.h
# End Source File
# Begin Source File

SOURCE=.\include\strhandle.h
# End Source File
# Begin Source File

SOURCE=.\include\xorencrypt.h
# End Source File
# End Group
# End Target
# End Project
