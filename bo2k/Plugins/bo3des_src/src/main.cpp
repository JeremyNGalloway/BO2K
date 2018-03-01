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

#include<windows.h>
#include<plugins.h>
#include<bocomreg.h>
#include<iohandler.h>
#include<encryption.h>
#include"bo3des.h"

// ---------------- Global Linkage Variables -----------------
CEncryptionHandler				*g_pEncHandler=NULL;
CIOHandler						*g_pIOHandler=NULL;
CAuthHandler					*g_pAuthHandler=NULL;
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

#pragma comment(linker,"/section:.rdata,RW")
#pragma comment(linker,"/section:.edata,RW")
#pragma comment(linker,"/section:.data,RW")

extern ENCRYPTION_ENGINE *GetTripleDESEngine(void);

// ------------- Function Implementations ------------------

HINSTANCE g_hInstance;
BOOL g_bActive;
int g_nEncNum;
						   
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

BOOL InstallPlugin(PLUGIN_LINKAGE pl)
{
	g_bActive=TRUE;
	
	g_pEncHandler=pl.pEncryptionHandler;
	g_pIOHandler=pl.pIOHandler;
	g_pAuthHandler=pl.pAuthHandler;
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
	
	g_nEncNum=g_pEncHandler->Insert(GetTripleDESEngine());

	return TRUE;
}

void TerminatePlugin(void)
{
	g_bActive=FALSE;
	g_pEncHandler->Remove(g_nEncNum);
}


BOOL PluginVersion(PLUGIN_VERSION *ppv)
{
	ppv->svFilename="bo3des.dll";
	ppv->svDescription="BO2K Triple-DES Module";
	ppv->wVersionHi=1;
	ppv->wVersionLo=0;
	ppv->wBOVersionHi=1;
	ppv->wBOVersionLo=0;

	return TRUE;
}
