# Microsoft Developer Studio Project File - Name="bo2kgui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=bo2kgui - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bo2kgui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bo2kgui.mak" CFG="bo2kgui - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bo2kgui - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "bo2kgui - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/bo2k/bo2kgui", MOBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bo2kgui - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "\projects\cj60lib\include" /I "include" /I "\projects\bo2k\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "__BO2KSERVER__" /FR /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 nafxcw.lib libcmt.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"nafxcw.lib libcmt.lib"

!ELSEIF  "$(CFG)" == "bo2kgui - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "\projects\cj60lib\include" /I "include" /I "\projects\bo2k\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "__BO2KSERVER__" /FR /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 nafxcwd.lib libcmtd.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"nafxcwd.lib libcmtd.lib" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no

!ENDIF 

# Begin Target

# Name "bo2kgui - Win32 Release"
# Name "bo2kgui - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "BO-Shared-Src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\auth\auth.cpp
# End Source File
# Begin Source File

SOURCE=..\src\commands\commnet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\misc\config\config.cpp
# End Source File
# Begin Source File

SOURCE=..\src\encryption\deshash.cpp
# End Source File
# Begin Source File

SOURCE=..\src\dll\dll_load.cpp
# End Source File
# Begin Source File

SOURCE=..\src\encryption\encryption.cpp
# End Source File
# Begin Source File

SOURCE=..\src\io\io_simpletcp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\io\io_simpleudp.cpp
# End Source File
# Begin Source File

SOURCE=..\src\io\iohandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\auth\nullauth.cpp
# End Source File
# Begin Source File

SOURCE=..\src\misc\osversion\osversion.cpp
# End Source File
# Begin Source File

SOURCE=..\src\libc\strhandle.cpp
# End Source File
# Begin Source File

SOURCE=..\src\encryption\xorencrypt.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\bo2kgui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BOCmdDescList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BODialogBar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BOServerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BOWDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BOWView.cpp
# End Source File
# Begin Source File

SOURCE=.\src\comctlmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\getcomctlversion.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GoodMenuBar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GradientDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\HistoryEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\InteractiveConnect.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\src\InteractiveListen.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\src\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\src\NewMachineDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pluginconfig.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\src\QueryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ServerList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Splash.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\trayicon.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "BO-Shared-Include"

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

SOURCE=..\include\comm_native.h
# End Source File
# Begin Source File

SOURCE=..\include\commnet.h
# End Source File
# Begin Source File

SOURCE=..\include\config.h
# End Source File
# Begin Source File

SOURCE=..\include\deshash.h
# End Source File
# Begin Source File

SOURCE=..\include\dll_load.h
# End Source File
# Begin Source File

SOURCE=..\include\encryption.h
# End Source File
# Begin Source File

SOURCE=..\include\io_simpletcp.h
# End Source File
# Begin Source File

SOURCE=..\include\io_simpleudp.h
# End Source File
# Begin Source File

SOURCE=..\include\iohandler.h
# End Source File
# Begin Source File

SOURCE=..\include\nullauth.h
# End Source File
# Begin Source File

SOURCE=..\include\osversion.h
# End Source File
# Begin Source File

SOURCE=..\include\plugins.h
# End Source File
# Begin Source File

SOURCE=..\include\strhandle.h
# End Source File
# Begin Source File

SOURCE=..\include\xorencrypt.h
# End Source File
# End Group
# Begin Group "CJ60Lib Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJ60Lib.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJCaption.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJControlBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJDockBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJDockContext.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJExplorerBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJFlatButton.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJFlatComboBox.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJFlatHeaderCtrl.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJFrameWnd.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJListCtrl.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJListView.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJMDIFrameWnd.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJMiniDockFrameWnd.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJOutlookBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJPagerCtrl.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJSearchEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJSizeDockBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJTabCtrlBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJTabView.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CJToolBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CoolBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\CoolMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\FixTB.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\FlatBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\HyperLink.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\MenuBar.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\ModulVer.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\ShellPidl.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\ShellTree.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\SHFileInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Include\Subclass.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\bo2kgui.h
# End Source File
# Begin Source File

SOURCE=.\include\BOCmdDescList.h
# End Source File
# Begin Source File

SOURCE=.\include\BODialogBar.h
# End Source File
# Begin Source File

SOURCE=.\include\BOServerDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\BOWDoc.h
# End Source File
# Begin Source File

SOURCE=.\include\BOWview.h
# End Source File
# Begin Source File

SOURCE=.\include\comctlmsg.h
# End Source File
# Begin Source File

SOURCE=.\include\getcomctlversion.h
# End Source File
# Begin Source File

SOURCE=.\include\GoodMenuBar.h
# End Source File
# Begin Source File

SOURCE=.\include\GradientDialog.h
# End Source File
# Begin Source File

SOURCE=.\src\HistoryEdit.h
# End Source File
# Begin Source File

SOURCE=.\include\InteractiveConnect.h
# End Source File
# Begin Source File

SOURCE=.\include\InteractiveListen.h
# End Source File
# Begin Source File

SOURCE=.\include\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\include\NewMachineDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\pluginconfig.h
# End Source File
# Begin Source File

SOURCE=.\include\QueryDlg.h
# End Source File
# Begin Source File

SOURCE=.\include\resource.h
# End Source File
# Begin Source File

SOURCE=.\include\ServerList.h
# End Source File
# Begin Source File

SOURCE=.\include\splash.h
# End Source File
# Begin Source File

SOURCE=.\include\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\include\trayicon.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\blue.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bo2k.BMP
# End Source File
# Begin Source File

SOURCE=.\res\bo2kgui.ico
# End Source File
# Begin Source File

SOURCE=.\bo2kgui.rc

!IF  "$(CFG)" == "bo2kgui - Win32 Release"

!ELSEIF  "$(CFG)" == "bo2kgui - Win32 Debug"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /i "include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\bo2kgui.rc2
# End Source File
# Begin Source File

SOURCE=.\res\bo2kstartup.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bocomput.ico
# End Source File
# Begin Source File

SOURCE=.\res\BOWDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\btn_arro.bmp
# End Source File
# Begin Source File

SOURCE=.\res\btn_explorer.bmp
# End Source File
# Begin Source File

SOURCE=.\res\button_images.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cdcbanner1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\connect.bmp
# End Source File
# Begin Source File

SOURCE=.\res\disconnect.bmp
# End Source File
# Begin Source File

SOURCE=.\res\editmach.ico
# End Source File
# Begin Source File

SOURCE=.\res\frame.bmp
# End Source File
# Begin Source File

SOURCE=.\res\green.bmp
# End Source File
# Begin Source File

SOURCE=.\res\hsplitba.cur
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\lit_bocomp.bmp
# End Source File
# Begin Source File

SOURCE=.\res\newmachi.ico
# End Source File
# Begin Source File

SOURCE=.\res\nrm_bocomp.bmp
# End Source File
# Begin Source File

SOURCE=.\res\optimg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\orange.bmp
# End Source File
# Begin Source File

SOURCE=.\res\red.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\treelist.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vectorball.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vectormask.bmp
# End Source File
# Begin Source File

SOURCE=.\res\vsplitba.cur
# End Source File
# Begin Source File

SOURCE=.\res\yellow.bmp
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\CJ60Lib\Lib\CJ60StaticLibd.lib

!IF  "$(CFG)" == "bo2kgui - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "bo2kgui - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\CJ60Lib\Lib\CJ60StaticLib.lib

!IF  "$(CFG)" == "bo2kgui - Win32 Release"

!ELSEIF  "$(CFG)" == "bo2kgui - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\bo2kgui.reg
# End Source File
# End Target
# End Project
