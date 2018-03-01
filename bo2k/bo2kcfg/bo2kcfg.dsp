# Microsoft Developer Studio Project File - Name="bo2kcfg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=bo2kcfg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bo2kcfg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bo2kcfg.mak" CFG="bo2kcfg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bo2kcfg - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "bo2kcfg - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/bo2k/bo2kcfg", UUCAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bo2kcfg - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 nafxcw.lib libcmt.lib /nologo /version:1.0 /subsystem:windows /machine:I386 /nodefaultlib:"nafxcw.lib libcmt.lib"

!ELSEIF  "$(CFG)" == "bo2kcfg - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 nafxcwd.lib libcmtd.lib /nologo /version:1.0 /subsystem:windows /debug /machine:I386 /nodefaultlib:"nafxcwd.lib libcmtd.lib" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bo2kcfg - Win32 Release"
# Name "bo2kcfg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Support Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\dll\dll_load.cpp
# End Source File
# Begin Source File

SOURCE=..\src\misc\osversion\osversion.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\bo2kcfg.cpp
# End Source File
# Begin Source File

SOURCE=.\bo2kcfgDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Wizard1.cpp
# End Source File
# Begin Source File

SOURCE=.\Wizard2.cpp
# End Source File
# Begin Source File

SOURCE=.\Wizard3.cpp
# End Source File
# Begin Source File

SOURCE=.\Wizard4.cpp
# End Source File
# Begin Source File

SOURCE=.\Wizard5.cpp
# End Source File
# Begin Source File

SOURCE=.\Wizard6.cpp
# End Source File
# Begin Source File

SOURCE=.\Wizard7.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Support Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\auth.h
# End Source File
# Begin Source File

SOURCE=..\include\bo_debug.h
# End Source File
# Begin Source File

SOURCE=..\include\bocomreg.h
# End Source File
# Begin Source File

SOURCE=..\include\dll_load.h
# End Source File
# Begin Source File

SOURCE=..\include\encryption.h
# End Source File
# Begin Source File

SOURCE=..\include\iohandler.h
# End Source File
# Begin Source File

SOURCE=..\include\osversion.h
# End Source File
# Begin Source File

SOURCE=..\include\plugins.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bo2kcfg.h
# End Source File
# Begin Source File

SOURCE=.\bo2kcfgDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Wizard1.h
# End Source File
# Begin Source File

SOURCE=.\Wizard2.h
# End Source File
# Begin Source File

SOURCE=.\Wizard3.h
# End Source File
# Begin Source File

SOURCE=.\Wizard4.h
# End Source File
# Begin Source File

SOURCE=.\Wizard5.h
# End Source File
# Begin Source File

SOURCE=.\Wizard6.h
# End Source File
# Begin Source File

SOURCE=.\Wizard7.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\banner1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bo2kcfg.ico
# End Source File
# Begin Source File

SOURCE=.\bo2kcfg.rc
# End Source File
# Begin Source File

SOURCE=.\res\bo2kcfg.rc2
# End Source File
# Begin Source File

SOURCE=.\res\bo2kcfgbanner.bmp
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\littleco.ico
# End Source File
# End Group
# End Target
# End Project
