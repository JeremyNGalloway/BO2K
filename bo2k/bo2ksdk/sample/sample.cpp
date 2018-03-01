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

#include<windows.h>
#include<plugins.h>
#include<bocomreg.h>
#include<iohandler.h>
#include<encryption.h>
#include"sample.h"

// Plugin Linkage Variables
CEncryptionHandler				*g_pEncHandler=NULL;
CIOHandler						*g_pIOHandler=NULL;
TYPEOF_RegisterCommand			*RegisterCommand=NULL;
TYPEOF_UnregisterCommand		*UnregisterCommand=NULL;
TYPEOF_RegisterClientMenu		*RegisterClientMenu=NULL;
TYPEOF_UnregisterClientMenu		*UnregisterClientMenu=NULL;
TYPEOF_IssueAuthCommandRequest	*IssueAuthCommandRequest=NULL;
TYPEOF_IssueAuthCommandReply	*IssueAuthCommandReply=NULL;
TYPEOF_ConnectAuthSocket		*ConnectAuthSocket=NULL;
TYPEOF_ListenAuthSocket			*ListenAuthSocket=NULL;
TYPEOF_InteractiveConnect		*InteractiveConnect=NULL;
TYPEOF_InteractiveListen		*InteractiveListen=NULL;

// Need this so that the configuration can be modified on the fly
#pragma comment(linker,"/section:.rdata,RW")
#pragma comment(linker,"/section:.data,RW")

// Global variables
HINSTANCE g_hInstance;
BOOL g_bActive;
long g_nNumThreads;
int g_nSampleCommand;						   

// Configuration string read by client,server and configuration tool
char g_szSampleOptions[]="<**CFG**>Sample\0"
			             "S[32]:Sample String Item=This is a string\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
                         "B:Sample Boolean Item=1\0"
						 "N[2,800]:Sample Numeric Item=69\0\0";


// ------------- Function Implementations ------------------

BOOL WINAPI DllMain(HINSTANCE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	// Do NOT perform configuration or initialization here

	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		g_hInstance=hInst;
		break;
	}
	return TRUE;
}

// InstallPlugin is called on both the server side and the client side to
// pass in function addresses that the plugin can use.
// Some functions are only passed in by the client, and some only by the
// server. If they are not passed, in they are NULL.

BOOL InstallPlugin(PLUGIN_LINKAGE pl)
{
	g_bActive=TRUE;
	g_nNumThreads=0;

	g_pEncHandler=pl.pEncryptionHandler;
	g_pIOHandler=pl.pIOHandler;
	RegisterCommand=pl.pRegisterCommand;
	UnregisterCommand=pl.pUnregisterCommand;
	IssueAuthCommandRequest=pl.pIssueAuthCommandRequest;
	IssueAuthCommandReply=pl.pIssueAuthCommandReply;
	ConnectAuthSocket=pl.pConnectAuthSocket;
	ListenAuthSocket=pl.pListenAuthSocket;
	RegisterClientMenu=pl.pRegisterClientMenu;
	UnregisterClientMenu=pl.pUnregisterClientMenu;
	InteractiveListen=pl.pInteractiveListen;
	InteractiveConnect=pl.pInteractiveConnect;
	
	// Server side command registration
	if(RegisterCommand) {
		g_nSampleCommand=RegisterCommand(CmdProc_SampleCommand,"Sample","Sample Command","Number Option",NULL,"String Option");
	}

	// Client side menu option registration
	if(RegisterClientMenu) {
		RegisterClientMenu("Sample","Sample Client",CreateSampleClient);
	}

	return TRUE;
}

// TerminatePlugin should wait for all threads spawned by the client or server
// to terminate, because the plugin DLL will be unloaded after this returns.

void TerminatePlugin(void)
{
	g_bActive=FALSE;
	while(g_nNumThreads>0) Sleep(0);

	if(UnregisterClientMenu) {
		UnregisterClientMenu("Sample","Sample Client");
	}

	if(UnregisterCommand) {
		UnregisterCommand(g_nSampleCommand);
	}

}

BOOL PluginVersion(PLUGIN_VERSION *ppv)
{
	ppv->svFilename="sample.dll";
	ppv->svDescription="BO2K SDK Sample Plugin";
	ppv->wVersionHi=0;
	ppv->wVersionLo=5;
	ppv->wBOVersionHi=1;
	ppv->wBOVersionLo=0;

	return TRUE;
}


// A sample server-side command

int CmdProc_SampleCommand(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{	
	// Do something here

	// Reply to client
	IssueAuthCommandReply(cas_from, comid, 1, "More text to come");
	IssueAuthCommandReply(cas_from, comid, 1, "More text to come");
	IssueAuthCommandReply(cas_from, comid, 1, "More text to come");
	IssueAuthCommandReply(cas_from, comid, 0, "Sample command executed");
	// IssueAuthCommandReply(cas_from, comid, 2, "Configuration changed, requery.");
	
	return 0;
}


// A sample client side menu option.
// Should create a thread and return immediately, as not to block client operation.

int CreateSampleClient(HWND hParent)
{
	/*
	DWORD dwtid;
	HANDLE htd;

	htd=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)SampleClientThread,(LPVOID)hParent,&dwtid);
	if(htd==NULL) {
		return -1;
	}
	*/

	return 0;
}
