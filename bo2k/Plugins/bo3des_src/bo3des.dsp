# Microsoft Developer Studio Project File - Name="bo3des" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=bo3des - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bo3des.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bo3des.mak" CFG="bo3des - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bo3des - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "bo3des - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/bo2k/bo3des", ETDAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bo3des - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BO3DES_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O1 /I "\projects\bo2k\bo3des\include" /I "\projects\bo2k\include" /I "\projects\bo2k\bo2kgui\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BO3DES_EXPORTS" /YX /Gs65536 /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:1.0 /entry:"DllMain@12" /dll /machine:I386 /nodefaultlib /def:".\bo3des.def"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "bo3des - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BO3DES_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "\projects\bo2k\bo3des\include" /I "\projects\bo2k\include" /I "\projects\bo2k\bo2kgui\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BO3DES_EXPORTS" /YX /Gs16384 /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /version:1.0 /entry:"DllMain@12" /dll /debug /machine:I386 /nodefaultlib /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bo3des - Win32 Release"
# Name "bo3des - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "support files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\misc\config\config.cpp
# End Source File
# Begin Source File

SOURCE=..\src\libc\fake_libc.c
# End Source File
# Begin Source File

SOURCE=..\src\libc\newdelete.cpp
# End Source File
# Begin Source File

SOURCE=..\src\libc\strhandle.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\3des.cpp
# End Source File
# Begin Source File

SOURCE=.\bo3des.def

!IF  "$(CFG)" == "bo3des - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "bo3des - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\md5.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "support headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\auth.h
# End Source File
# Begin Source File

SOURCE=..\include\bocomreg.h
# End Source File
# Begin Source File

SOURCE=..\include\comm_native.h
# End Source File
# Begin Source File

SOURCE=..\include\config.h
# End Source File
# Begin Source File

SOURCE=..\include\encryption.h
# End Source File
# Begin Source File

SOURCE=..\include\iohandler.h
# End Source File
# Begin Source File

SOURCE=..\include\plugins.h
# End Source File
# Begin Source File

SOURCE=..\include\strhandle.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\bo3des.h
# End Source File
# Begin Source File

SOURCE=.\include\global.h
# End Source File
# Begin Source File

SOURCE=.\include\md5.h
# End Source File
# End Group
# End Target
# End Project
